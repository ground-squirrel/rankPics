#include "QueryCollection.h"
#include <boost/filesystem.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <iostream>

QueryCollection::QueryCollection( std::string indexFramesPath, std::string queryFramePath )
        : mIndexFramesPath( indexFramesPath )
        , mQueryFramePath( queryFramePath )
        , mpIdAndPathMap ( new IdAndPathMap() )
        , mpRanks( new CollectionRanks() )
        , mpDescriptors( new CollectionDescriptors() )
{
}

QueryCollection::~QueryCollection()
{
        if( mpIdAndPathMap )
        {
                delete mpIdAndPathMap;
        }
        if( mpRanks )
        {
                delete mpRanks;
        }
        if( mpDescriptors )
        {
            delete mpDescriptors;
        }
}

bool QueryCollection::comparePathAndRankPairs( const IdAndRank & leftPair, const IdAndRank & rightPair )
{
    return ( leftPair.second < rightPair.second );
}

bool QueryCollection::processCollection()
{
        namespace fs = boost::filesystem;

        fs::path IndexFramesPath( mIndexFramesPath );

        //проверяем путь к коллекции кадров
        if( !fs::exists( IndexFramesPath ) || !fs::is_directory( IndexFramesPath ) )
        {
                std::cerr << "Неверный путь к коллекции кадров" << std::endl;
                return false;
        }

        unsigned long file_count = 0;
        fs::directory_iterator end_iter;

        for( fs::directory_iterator dir_iter( IndexFramesPath ); dir_iter != end_iter; dir_iter++  )
        {
                try
                {
                        if ( fs::is_regular_file( dir_iter->status() ) )
                        {
                                ++file_count;
                                mpIdAndPathMap->emplace( file_count, std::string( dir_iter->path().c_str() ) );

                                //обработка изображения
                                processImage( file_count );
                        }
                }
                catch ( const std::exception & ex )
                {
                        std::cout << dir_iter->path().filename() << " " << ex.what() << std::endl;
                        return false;
                }

                if( 0 == file_count )
                {
                        std::cout << "Директория с коллекцией кадров пуста" << std::endl;
                        return false;
                }
        }

        return true;
}

 /* Ранжирование коллекции по отношению к кадру-запросу */
void QueryCollection::rank()
{
        if( !processQueryImage() )
        {
                return;
        }

        if( 0 == mpDescriptors->size() )
        {
                if( !processCollection() )
                {
                         return;
                }
        }

        //вычисляем ранг кадров коллекции по отношению к кадру-запросу
        for( unsigned long i = 0; i < mpDescriptors->size(); i++)
        {
                double rank = calculateImageRank( mpDescriptors->at(i).second );
                unsigned long imageId = mpDescriptors->at(i).first;
                if( rank >= 0 )
                {
                        mpRanks->emplace_back( IdAndRank( imageId, rank ) );
                }
                else
                {
                        std::cout << "Invalid rank " << mpIdAndPathMap->at(imageId) << std::endl;
                }
        }

        //cортируем кадры коллекции по рангу
        mpRanks->sort( comparePathAndRankPairs );

        //выводим отсортированный список
        std::cout << "\nСписок кадров коллекции в порядке убывания ранга" << std::endl;
        for( CollectionRanks::const_iterator it = mpRanks->cbegin(); it != mpRanks->cend(); it++ )
        {
            unsigned long imageId = (*it).first;
            std::cout << (*it).second << mpIdAndPathMap->at(imageId) << std::endl;
        }

         //выводим кадр коллекции с максимальным рангом
        if( mpRanks->size() > 0 )
        {
                unsigned long minRankImageId = mpRanks->front().first;
                //получаем полный путь к кадру с минимальным рангом
                std::string minRankImagePath ( mpIdAndPathMap->at( minRankImageId ) );

                std::cout << "\nИзображение с максимальным рангом " << minRankImagePath << std::endl;

                //считываем кадр
                cv::Mat minRankImage = cv::imread( minRankImagePath, CV_LOAD_IMAGE_COLOR );

                //проверяем
                if(! minRankImage.data )
                {
                        std::cout <<  "Невозможно открыть файл "  << minRankImagePath << std::endl ;
                }
                else
                {
                        cv::namedWindow( "Изображение с максимальным рангом", cv::WINDOW_AUTOSIZE );
                        cv::imshow( "Изображение с максимальным рангом", minRankImage );

                        cv::waitKey(0);
                }
        }
        else
        {
                std::cout << "Похожих изображений в коллекции не найдено" << std::endl;
        }
}

/* Обработка изображения и извлечение дескриптора */
void QueryCollection::processImage( unsigned long imageId )
{
        std::string imagePath = mpIdAndPathMap->at(imageId);
        //загружаем изображение
        cv::Mat indexFrame = cv::imread( imagePath, CV_LOAD_IMAGE_GRAYSCALE );

        //проверяем
        if( !indexFrame.data )
        {
                std::cout <<  "Невозможно открыть файл " << imagePath << std::endl;
        }
        else
        {
                cv::Mat imagePreprocessed;

                preprocessImage( indexFrame, &imagePreprocessed );

                // Выделяем ключевые точки с помощью SURF-детектора
                int minHessian = 400;
                int nOctaves = 4;

                cv::SurfFeatureDetector detector( minHessian, nOctaves );

                std::vector<cv::KeyPoint> keypoints_image;

                detector.detect( imagePreprocessed, keypoints_image );

                if( keypoints_image.empty() )
                {
                        std::cout << "Невозможно определить ключевые точки для кадра " << imagePath << std::endl;
                        return;
                }

                //Извлекаем дескрипторы
                cv::SurfDescriptorExtractor extractor;

                cv::Mat descriptors_image;

                extractor.compute( imagePreprocessed, keypoints_image, descriptors_image );

                if( descriptors_image.empty() )
                {
                        std::cout << "Невозможно извлечь дескрипторы для кадра " << imagePath << std::endl;
                        return;
                }

                mpDescriptors->emplace_back( IdAndDescriptors( imageId, descriptors_image ) );
        }
}

/* Вычисление ранга для пар кадр-запрос и кадр из коллекции */
double QueryCollection::calculateImageRank( const cv::Mat & rImageDescriptors )
{
         //сравниваем дескрипторы текущего кадра и кадра-запроса с помощью FLANN matcher
        cv::FlannBasedMatcher matcher;
        std::vector< cv::DMatch > matches;
        matcher.match( mDescriptorsQuery, rImageDescriptors, matches );

        double rank = 0;
        //вычисляем ранг как сумму расстояний между keypoints
        for( int i = 0; i < mDescriptorsQuery.rows; i++ )
        {
                rank += matches[i].distance;
        }

        return rank;
}

bool QueryCollection::processQueryImage()
{
        namespace fs = boost::filesystem;

        fs::path IndexFramesPath( mQueryFramePath );

        //проверяем путь к кадру-запросу
        if( !fs::exists( mQueryFramePath ) )
        {
                std::cerr << "Неверный путь к кадру-запросу" << std::endl;
                return false;
        }

        std::string queryFrameFullPath ( mQueryFramePath );

        if( fs::is_directory( IndexFramesPath ) )
        {

                unsigned long file_count = 0;
                fs::directory_iterator end_iter;
                for( fs::directory_iterator dir_iter( IndexFramesPath ); dir_iter != end_iter; dir_iter++ )
                {
                        try
                        {
                                if ( fs::is_regular_file( dir_iter->status() ) )
                                {
                                        ++file_count;
                                        queryFrameFullPath = std::string( dir_iter->path().c_str() );
                                        break;
                                }
                        }
                        catch ( const std::exception & ex )
                        {
                                std::cout << dir_iter->path().filename() << " " << ex.what() << std::endl;
                        }
                }
                if( 0 == file_count )
                {
                         std::cerr << "Неверный путь к кадру-запросу" << std::endl;
                         return false;
                }
        }

        //считываем кадр-запрос
        cv::Mat queryImage = cv::imread( queryFrameFullPath, CV_LOAD_IMAGE_GRAYSCALE );

        //проверяем
        if( !queryImage.data )
        {
                std::cout <<  "Невозможно открыть файл с кадром-запросом " << queryFrameFullPath << std::endl;
                return false;
        }

        cv::Mat imagePreprocessed;
        preprocessImage( queryImage, &imagePreprocessed );

        int minHessian = 400;
        int nOctaves = 4;

        cv::SurfFeatureDetector detector( minHessian, nOctaves );

        std::vector<cv::KeyPoint> keyPointsQuery;

        detector.detect( imagePreprocessed, keyPointsQuery );

        cv::SurfDescriptorExtractor extractor;

        extractor.compute( imagePreprocessed, keyPointsQuery, mDescriptorsQuery );

        if( mDescriptorsQuery.empty() )
        {
                cvError( 0,"processQueryImage","Query descriptor is empty",__FILE__,__LINE__ );
                return false;
        }

        return true;
}

/* Предобработка кадра */
void QueryCollection::preprocessImage( const cv::Mat & rSource, cv::Mat * pDest )
{
    cv::GaussianBlur( rSource, *pDest, cv::Size( 5, 5 ), 0, 0 );
}
