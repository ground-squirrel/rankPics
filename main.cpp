/* 
 * Ранжирование коллекции кадров (директория index_frames)
 * по отношению к кадру-запрос (директория query_frame).
 * Максимальный ранг присваивается кадру, наиболее похожему на кадр-запрос.
 */

#include "QueryCollection.h"
#include <iostream>
#include <string>
#include <boost/program_options.hpp>

using namespace std;


int main(int argc, char** argv) 
{
        locale::global( locale( "" ) );        
    
        namespace po = boost::program_options;

        po::options_description desc("опции");
        desc.add_options()
            ("help,h", "помощь")
            ("index,i", po::value<std::string>(), "путь к коллекции кадров для ранжирования")
            ("query,q", po::value<std::string>(), "путь к кадру-запросу")
        ;

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);    

        string indexFramesPath, queryFramePath;

        if ( !vm.size() || vm.count( "help" ) )
        {
                cout << "\nИспользование:\n"
                        "./rankcollection -i [--index] <index_frames_dirpath> "
                        "-q [--query] <query_frame_dirpath>\n" << endl;
                cout << desc << endl;
                return 1;
        }

        if ( vm.count( "index" ) )
        {
                indexFramesPath = vm["index"].as<std::string>();
        }
        else
        {
                cerr << "Не задан путь к коллекции кадров для ранжирования" << endl;
                return 1;
        }

        if( vm.count( "query" ) )
        {
                queryFramePath = vm["query"].as<std::string>();
        }
        else
        {
                cerr << "Не задан путь к кадру-запросу" << endl;
                return 1;
        }

        QueryCollection lCollection( indexFramesPath, queryFramePath );
        lCollection.rank();
        
        return 0;
}

