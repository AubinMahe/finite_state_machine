#include <famo/entrées_sorties_via_fichier_texte.hpp>
#include <famo/four_à_micro_ondes.hpp>

#include <cstdlib>

int main( int argc, char * argv[] ) {
   const char * in_path  = "four à micro-ondes-in.txt";
   const char * out_path = "four à micro-ondes-out.txt";
   if( argc > 1 ) {
      in_path = argv[1];
      if( argc > 2 ) {
         out_path = argv[2];
      }
   }
   famo::entrées_sorties_via_fichier_texte intrfc( in_path, out_path );
   famo::four_à_micro_ondes four( intrfc );
   four.run();
   return EXIT_SUCCESS;
}
