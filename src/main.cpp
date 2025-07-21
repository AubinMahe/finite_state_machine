#include <famo/entrées_sorties_via_fichier_texte.hpp>
#include <famo/entrées_sorties_via_web_socket.hpp>
#include <famo/four_à_micro_ondes.hpp>

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

static const char           in_path [] = "four à micro-ondes-in.txt";
static const char           out_path[] = "four à micro-ondes-out.txt";
static const unsigned short port       = 2416;

static int usage( const char * exename, const std::string & message = "" ) {
   std::cerr
      << "usage: " << exename << " [-h] [-t in out] | [-w port] "                           << std::endl
      << "\t-h: display this help"                                                          << std::endl
      << "\t-v: verbose mode"                                                               << std::endl
      << "\t-t: text interface wicth in and out pathes to text files"                       << std::endl
      << "\t-w: web interface with port of local web socket"                                << std::endl
      << "\tdefault is text, with in = '" << in_path << "' ans our is '" << out_path << '"' << std::endl;
   if( message.length()) {
      std::cerr << message << std::endl;
   }
   return EXIT_FAILURE;
}

int main( int argc, char * argv[] ) {
   // std::unique_ptr<famo::entrées_sorties> intrfc;
   famo::entrées_sorties * intrfc = 0;
   bool verbose = false;
   for( int i = 1; i < argc; ++i ) {
      std::string arg( argv[i]);
      if( arg == "-h" ) {
         return usage( argv[0] );
      }
      if( arg == "-v" ) {
         verbose = true;
      }
      else if( arg == "-t" ) {
         if( argc > i+2 ) {
            std::error_code rc;
            std::filesystem::path in = std::filesystem::absolute( argv[++i], rc );
            if( rc ) {
               return usage( argv[0], ( std::string( "bad path: '" ) + argv[i]) + "'" );
            }
            std::filesystem::path out = std::filesystem::absolute( argv[++i], rc );
            if( rc ) {
               return usage( argv[0], ( std::string( "bad path: '" ) + argv[i]) + "'" );
            }
            // intrfc = std::unique_ptr<famo::entrées_sorties>( new famo::entrées_sorties_via_fichier_texte( in.c_str(), out.c_str()));
            intrfc = new famo::entrées_sorties_via_fichier_texte( in.c_str(), out.c_str());
         }
         else {
            return usage( argv[0], "-t expect two arguments" );
         }
      }
      else if( arg == "-w" ) {
         if( argc > i+1 ) {
            try {
               unsigned short port = static_cast<unsigned short>( std::stoul( argv[++i]));
               // intrfc = std::unique_ptr<famo::entrées_sorties>( new famo::entrées_sorties_via_web_socket( port ));
               intrfc = new famo::entrées_sorties_via_web_socket( port );
            }
            catch( const std::out_of_range & ) {
               return usage( argv[0], std::string( "-w: port value is out of range: " ) + argv[i]);
            }
            catch( const std::invalid_argument & ) {
               return usage( argv[0], std::string( "-w: port value is not an unsigned short integer: '" ) + argv[i] + "'" );
            }
         }
         else {
            return usage( argv[0], "-w expect one argument" );
         }
      }
      else {
         return usage( argv[0], std::string( "unexpected argument: " ) + arg );
      }
   }
   if( intrfc == 0 ) {
      // intrfc = std::unique_ptr<famo::entrées_sorties>( new famo::entrées_sorties_via_fichier_texte( in_path, out_path ));
      intrfc = new famo::entrées_sorties_via_fichier_texte( in_path, out_path );
   }
   famo::four_à_micro_ondes four( *intrfc, verbose );
   four.run();
   delete intrfc;
   return EXIT_SUCCESS;
}
