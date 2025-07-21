#include <websocket_and_http_servers.hpp>

#include <nlohmann/json.hpp>

#include <iostream>
#include <stdexcept>
#include <thread>

websocket_and_http_servers::websocket_and_http_servers( unsigned short port ) {
   static lws_protocols             protocols[3];
   static lws_http_mount            mount;
   static lws_context_creation_info info;
   mount.mountpoint      = "/";
   mount.mountpoint_len  = 1;
   mount.origin          = "/home/aubin/Dev/git/finite_state_machine/pages";
   mount.def             = "index.html";
   mount.origin_protocol = LWSMPRO_FILE;
   protocols[0].name     = "http";
   protocols[0].callback = lws_callback_http_dummy;
   protocols[1].name     = "fsm";
   protocols[1].callback = (lws_callback_function *)websocket_and_http_servers::lws_callback;
   info.port             = port;
   info.mounts           = &mount;
   info.protocols        = protocols;
   info.vhost_name       = "localhost";
   info.options          = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;
   info.user             = this;
   lws_context * ctxt = lws_create_context( &info );
   if( ! ctxt ) {
      throw std::logic_error( "web socket init failed" );
   }
   std::thread([this, ctxt](){
      while( lws_service( ctxt, 20 ) == 0 ) {
         if( _message_to_send.length()) {
            lws_callback_on_writable_all_protocol( ctxt, &protocols[1] );
         }
      }
      lws_context_destroy( ctxt );
   }).detach();
}

void websocket_and_http_servers::write( lws * wsi ) {
   static unsigned char buffer[4000];
   size_t len = _message_to_send.length();
   if( len < sizeof( buffer )) {
      memcpy( buffer, _message_to_send.c_str(), len + 1 );
      lws_write( wsi, buffer, len, LWS_WRITE_TEXT );
   }
   else {
      std::cerr
         << "websocket_and_http_servers::write|Message trop grand !"
         << std::endl;
   }
   _message_to_send = "";
}

void websocket_and_http_servers::receive( const std::string & json_data ) {
   try {
      nlohmann::json json = nlohmann::json::parse( json_data );
      _la_porte_est_ouverte                = json["la_porte_est_ouverte"];
      _la_valeur_du_minuteur_en_ms         = json["la_valeur_du_minuteur_en_ms"];
      _le_four_est_définitivement_en_panne = json["le_four_est_définitivement_en_panne"];
   }
   catch( const nlohmann::json::parse_error & pe ) {
      std::cerr
         << "websocket_and_http_servers::receive|nlohmann::json::parse_error: "
         << pe.what()
         << std::endl;
   }
}

int websocket_and_http_servers::lws_callback( lws * wsi, lws_callback_reasons reason, void *, char * data, size_t data_length ) {
   lws_context * context = lws_get_context( wsi );
   if( context ) {
      websocket_and_http_servers * This = (websocket_and_http_servers *)lws_context_user( context );
      if( This ) {
         if( reason == LWS_CALLBACK_SERVER_WRITEABLE ) {
            This->write( wsi );
         }
         else if( reason == LWS_CALLBACK_RECEIVE ) {
            data[data_length] = '\0';
            This->receive( data );
         }
      }
      else {
         std::cerr
            << "websocket_and_http_servers::lws_callback|This est nul !"
            << std::endl;
      }
   }
   return 0;
}

bool websocket_and_http_servers::send_message( const std::string & message ) {
   _message_to_send = message;
   return true;
}

bool websocket_and_http_servers::get_states(
   bool & la_porte_est_ouverte,
   long & la_valeur_du_minuteur_en_ms,
   bool & le_four_est_définitivement_en_panne )
{
   la_porte_est_ouverte                = _la_porte_est_ouverte;
   la_valeur_du_minuteur_en_ms         = _la_valeur_du_minuteur_en_ms;
   le_four_est_définitivement_en_panne = _le_four_est_définitivement_en_panne;
   return true;
}
