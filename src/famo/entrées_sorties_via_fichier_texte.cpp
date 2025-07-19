#include <famo/entrées_sorties_via_fichier_texte.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

using namespace famo;

entrées_sorties_via_fichier_texte::entrées_sorties_via_fichier_texte( const char * in_path, const char * out_path ) :
   _in_path ( in_path  ),
   _out_path( out_path )
{
   std::ofstream( _in_path, std::ios::out )
      << "0 0 0"
      << std::endl;
}

 bool entrées_sorties_via_fichier_texte::obtenir(
   bool & la_porte_est_ouverte,
   long & la_valeur_du_minuteur_en_ms,
   bool & l_état_de_panne_définitive )
{
   static std::filesystem::file_time_type p_lwt;
   std::error_code err;
   std::filesystem::file_time_type lwt = std::filesystem::last_write_time( _in_path, err );
   if( err ) {
      std::cerr << err.message() << std::endl;
      return false;
   }
   if( lwt.time_since_epoch().count() != p_lwt.time_since_epoch().count()) {
      p_lwt = lwt;
      std::ifstream( _in_path )
         >> la_porte_est_ouverte
         >> la_valeur_du_minuteur_en_ms
         >> l_état_de_panne_définitive;
      // std::cout
      //    << "Lecture de '" << _in_path << "'" << std::endl
      //    << "\tporte ouverte        : " << la_porte_est_ouverte        << std::endl
      //    << "\tminuteur             : " << la_valeur_du_minuteur_en_ms << std::endl
      //    << "\ten panne irréparable : " << l_état_de_panne_définitive  << std::endl;
   }
   return true;
}

void entrées_sorties_via_fichier_texte::publier(
   const std::string & le_nom_logique_de_l_état_de_l_automate,
   bool la_lumière_est_allumée,
   bool la_porte_est_ouverte,
   long la_valeur_du_minuteur_en_ms,
   bool le_four_est_définitivement_en_panne )
{
   static std::string p_states = "";
   std::stringstream ss;
   ss << "Automate : " << le_nom_logique_de_l_état_de_l_automate                                      << std::endl
      << "Lumière  : " << ( la_lumière_est_allumée              ? "allumée"              : "éteinte") << std::endl
      << "Porte    : " << ( la_porte_est_ouverte                ? "ouverte"              : "fermée" ) << std::endl
      << "État     : " << ( le_four_est_définitivement_en_panne ? "en panne irréparable" : "en état") << std::endl
      << "Minuteur : " << la_valeur_du_minuteur_en_ms << " ms"                                        << std::endl;
   std::string states = ss.str();
   if( states != p_states ) {
      p_states = states;
      std::ofstream( _out_path ) << states;
      // std::cout
      //    << "Écriture de '" << _out_path << "'" << std::endl
      //    << states;
   }
}
