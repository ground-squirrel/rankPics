#ifndef QUERYCOLLECTION_H
#define	QUERYCOLLECTION_H

#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <string>
#include <list>
#include <map>
#include <utility>


class QueryCollection
{
public:
         /* идентификатор кадра и путь к нему */
         typedef std::map< unsigned long, std::string > IdAndPathMap;
        /* идентификатор кадра и его ранг */
        typedef std::pair< unsigned long, double > IdAndRank;
        typedef std::list< IdAndRank > CollectionRanks;
        /* идентификатор кадра и его дескрипторы */
        typedef std::pair< unsigned long, cv::Mat > IdAndDescriptors;
        typedef std::vector< IdAndDescriptors > CollectionDescriptors;

private:
        /* путь к коллекции кадров */
        std::string mIndexFramesPath;
        /* путь к кадру-запросу */
        std::string mQueryFramePath;
        /* идентификаторы кадров и пути к ним */
        IdAndPathMap * mpIdAndPathMap;
        /* имя кадра и его ранг по отношению к кадру-запросу */
        CollectionRanks * mpRanks;
        /* дескрипторы кадров коллекции */
        CollectionDescriptors * mpDescriptors;
        /* дескрипторы кадра-запроса */
        cv::Mat mDescriptorsQuery;

private:
        QueryCollection& operator=( const QueryCollection & other );
        QueryCollection( const QueryCollection & other );

protected:
        /* Обработка изображения и извлечение дескриптора */
        void processImage( unsigned long imageId );
        /* Предобработка кадра */
        void preprocessImage( const cv::Mat & rSource, cv::Mat * pDest );
        /* Вычисление ранга для пар кадр-запрос и кадр из коллекции.
         * Возвращает -1 в случае ошибки
         */
        double calculateImageRank( const cv::Mat & rImageDescriptors );
        /* Обработка кадра-запроса */
        bool processQueryImage();
        /* Предобработка коллекции кадров */
        bool processCollection();
        /* Вычисление рангов всех кадров коллекции */
        void calculateImageRanks();
        /* фунция сравнения для сортировки CollectionRanks */
        static bool comparePathAndRankPairs( const IdAndRank & first, const IdAndRank & second );

public:
        QueryCollection( std::string indexFramesPath, std::string queryFramePath );
        ~QueryCollection();

        /* Ранжирование коллекции по отношению к кадру-запросу */
        void rank();
};

#endif	/* QUERYCOLLECTION_H */

