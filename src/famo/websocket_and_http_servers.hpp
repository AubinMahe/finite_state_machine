#pragma once
#include <libwebsockets.h>
#include <string>

class websocket_and_http_servers {
public:

   websocket_and_http_servers( unsigned short port );

public:

   bool send_message( const std::string & message );
   bool get_states(
      bool & la_porte_est_ouverte,
      long & la_valeur_du_minuteur_en_ms,
      bool & le_four_est_définitivement_en_panne );

private:

   void write( lws * wsi );

   void receive( const std::string & json_data );

   static int lws_callback(
      lws                  * wsi,
      lws_callback_reasons   reason,
      void                 * unused,
      char                 * data,
      size_t                 data_length );

private:

   std::string _message_to_send;
   bool        _la_porte_est_ouverte = false;
   long        _la_valeur_du_minuteur_en_ms = 0L;
   bool        _le_four_est_définitivement_en_panne = false;

   websocket_and_http_servers( const websocket_and_http_servers & ) = delete;
   websocket_and_http_servers & operator = ( const websocket_and_http_servers & ) = delete;
};
