#include <famo/entrées_sorties_via_web_socket.hpp>
#include "websocket_and_http_servers.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <thread>

using namespace famo;

entrées_sorties_via_web_socket::entrées_sorties_via_web_socket( unsigned short port ) :
   _ws_server( new websocket_and_http_servers( port ))
{}

entrées_sorties_via_web_socket:: ~ entrées_sorties_via_web_socket( void ) {
   delete _ws_server;
}

bool entrées_sorties_via_web_socket::obtenir(
   bool & la_porte_est_ouverte,
   long & la_valeur_du_minuteur_en_ms,
   bool & le_four_est_définitivement_en_panne )
{
   la_porte_est_ouverte = false;
   la_valeur_du_minuteur_en_ms = 0L;
   le_four_est_définitivement_en_panne = false;
   return _ws_server && _ws_server->get_states(
      la_porte_est_ouverte,
      la_valeur_du_minuteur_en_ms,
      le_four_est_définitivement_en_panne );
}

static std::ostream & kv( std::ostream & json, const std::string & key, const std::string & value ) {
   return json << '"' << key << '"' << ':' << '"' << value << '"';
}

static std::ostream & kv( std::ostream & json, const std::string & key, bool value ) {
   return json << '"' << key << '"' << ':' << ( value ? "true" : "false" );
}

static std::ostream & kv( std::ostream & json, const std::string & key, long value ) {
   return json << '"' << key << '"' << ':' << value;
}

bool entrées_sorties_via_web_socket::publier(
   const std::string & le_nom_logique_de_l_état_de_l_automate,
   bool la_lumière_est_allumée,
   bool la_porte_est_ouverte,
   long la_valeur_du_minuteur_en_ms,
   bool le_four_est_définitivement_en_panne )
{
   std::stringstream ss;
   ss << "{";
   kv( ss, "état_courant"                       , le_nom_logique_de_l_état_de_l_automate ) << ",";
   kv( ss, "la_lumière_est_allumée"             , la_lumière_est_allumée                 ) << ",";
   kv( ss, "la_porte_est_ouverte"               , la_porte_est_ouverte                   ) << ",";
   kv( ss, "la_valeur_du_minuteur_en_ms"        , la_valeur_du_minuteur_en_ms            ) << ",";
   kv( ss, "le_four_est_définitivement_en_panne", le_four_est_définitivement_en_panne    ) << "}" << std::endl;
   std::string message = ss.str();
   return _ws_server->send_message( message );
}
