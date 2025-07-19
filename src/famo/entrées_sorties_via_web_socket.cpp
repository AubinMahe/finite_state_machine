#include <famo/entrées_sorties_via_web_socket.hpp>

#include <iostream>

using namespace famo;

entrées_sorties_via_web_socket::entrées_sorties_via_web_socket( unsigned short port ) :
   _port( port )
{

}

bool entrées_sorties_via_web_socket::obtenir(
   bool & la_porte_est_ouverte,
   long & la_valeur_du_minuteur_en_ms,
   bool & l_état_de_panne_définitive )
{
   la_porte_est_ouverte = false;
   la_valeur_du_minuteur_en_ms = 0L;
   l_état_de_panne_définitive = false;
   return true;
}

void entrées_sorties_via_web_socket::publier(
   const std::string & le_nom_logique_de_l_état_de_l_automate,
   bool la_lumière_est_allumée,
   bool la_porte_est_ouverte,
   long la_valeur_du_minuteur_en_ms,
   bool le_four_est_définitivement_en_panne )
{
   std::cout
      << le_nom_logique_de_l_état_de_l_automate
      << la_lumière_est_allumée
      << la_porte_est_ouverte
      << la_valeur_du_minuteur_en_ms
      << le_four_est_définitivement_en_panne;
}
