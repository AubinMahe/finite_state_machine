#include <fsm.hpp>
#include <iso_iec_6429.hpp>

#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>

enum class state_t {
   NONE = 0,
   ATTENTE,
   CHAUFFE, // Rotation du plateau, émission des micro-ondes, décompte du temps
   HORS_D_USAGE,
};

static std::ostream & operator << ( std::ostream & stream, const state_t & state ) {
   switch( state ) {
      case state_t::NONE        : return stream << "NONE";
      case state_t::ATTENTE     : return stream << "ATTENTE";
      case state_t::CHAUFFE     : return stream << "CHAUFFE";
      case state_t::HORS_D_USAGE: return stream << "HORS_D_USAGE";
   }
   return stream << "???";
}

enum class event_t {
   NONE = 0,
   PORTE_OUVERTE,
   PORTE_FERMÉE,
   MINUTEUR_POSITIF_ET_PORTE_FERMÉE,
   TEMPS_ÉCOULÉ,
   PANNE_IRRÉPARABLE
};

/**
 * Légende
 *=========
 *    ╔═══════════════════╗
 *    ║ ACTIVITÉ INITIALE ║
 *    ╚═══════════════════╝
 *    ┌──────────┐
 *    │ ACTIVITÉ │ Une activité prend du temps, elle dure.
 *    └──────────┘
 *    (événement)  Un événement est généré par consultation de l'état du système, ce dernier pouvant être modifié par une action extérieure (humaine en l'occurence).
 *    [action]     Une action est ponctuelle, sa durée est négligeable.
 * 
 * Automate d'états
 *==================
 *        ╔═══════════╗
 *    ┌──>║  ATTENTE  ╟──────────────>────────────────────┬────>────┬──────>───────┐
 *    │   ╚═╤═══╤═══╤═╝                                   │         │              │
 *    │     │   │   │ (Porte fermée)[Éteindre la lumière] │         │              │
 *    │     │   │   └───────────────<─────────────────────┘         │              │
 *    │     │   │                                                   │              │
 *    │     │   │        (Porte ouverte)[Allumer la lumière]        │              │
 *    │     │   └─────────────────────<────────────────────────<────┴───<──┐       │
 *    │     │                                                              │       │
 *    │     │ ((minuteur > 0) et (Porte fermée))[Allumer la lumière]   ┌───┴─────┐ │
 *    │     └────────────────>─────────────────>──────────────────────>│ CHAUFFE │ │
 *    │                                                                └─┬───┬───┘ │
 *    │                              (Temps écoulé)[Éteindre la lumière] │   │     │
 *    └──────────────<─────────────────────────────<─────────────────────┘   │     │
 *                                                                           │     │
 *         ┌──────────────┐                 (En panne irréparable)[Recycler] │     │
 *         │ HORS D'USAGE ├─────────────<─────────────────────<──────────────┴───<─┘
 *         └──────────────┘
 *
 * <https://cloford.com/resources/charcodes/utf-8_box-drawing.htm>
*/
class four_a_micro_onde :
   public hpms::fsm<state_t, event_t>
{
public:

   four_a_micro_onde( const char * in_path, const char * out_path ) :
      hpms::fsm<state_t, event_t>( state_t::ATTENTE, {
         {state_t::ATTENTE, event_t::PORTE_OUVERTE                    , state_t::ATTENTE     , [this](){ allumer_la_lumière();  }},
         {state_t::ATTENTE, event_t::PORTE_FERMÉE                     , state_t::ATTENTE     , [this](){ éteindre_la_lumière(); }},
         {state_t::ATTENTE, event_t::MINUTEUR_POSITIF_ET_PORTE_FERMÉE , state_t::CHAUFFE     , [this](){ allumer_la_lumière();  }},
         {state_t::ATTENTE, event_t::PANNE_IRRÉPARABLE                , state_t::HORS_D_USAGE, [this](){ recycler();            }},
         {state_t::CHAUFFE, event_t::TEMPS_ÉCOULÉ                     , state_t::ATTENTE     , [this](){ éteindre_la_lumière(); }},
         {state_t::CHAUFFE, event_t::PORTE_OUVERTE                    , state_t::ATTENTE     , [this](){ allumer_la_lumière();  }},
         {state_t::CHAUFFE, event_t::PANNE_IRRÉPARABLE                , state_t::HORS_D_USAGE, [this](){ recycler();            }},
      }),
      _in_path ( in_path  ),
      _out_path( out_path )
   {
      set_activity( state_t::CHAUFFE, [this](){ chauffer(); });
      std::ofstream( _in_path, std::ios::out )
         << "0 0 0"
         << std::endl;
   }

protected:

   virtual void acquire_sensors_statuses( void ) {
      static std::filesystem::file_time_type p_lwt;
      static std::string                     p_states = "";
      std::filesystem::file_time_type lwt = std::filesystem::last_write_time( _in_path );
      if( lwt.time_since_epoch().count() != p_lwt.time_since_epoch().count()) {
         p_lwt = lwt;
         const std::lock_guard<std::mutex> lock( _consignes_mutex );
         std::ifstream( _in_path )
            >> _porte_ouverte
            >> _minuteur_ms
            >> _en_panne_irréparable;
         std::cout
            << "Lecture de " << _in_path << std::endl
            << "\tporte ouverte        : " << _porte_ouverte        << std::endl
            << "\tremaining            : " << _minuteur_ms          << std::endl
            << "\ten panne irréparable : " << _en_panne_irréparable << std::endl;
      }
      std::stringstream ss;
      ss << "Automate : " << get_current_state() << std::endl
         << "Lumière  : " << ( _lumière_allumée      ? "allumée"              : "éteinte") << std::endl
         << "Porte    : " << ( _porte_ouverte        ? "ouverte"              : "fermée" ) << std::endl
         << "État     : " << ( _en_panne_irréparable ? "en panne irréparable" : "en état") << std::endl
         << "Remaining: " << _minuteur_ms << std::endl;
      std::string states = ss.str();
      if( states != p_states ) {
         std::ofstream( _out_path ) << states;
         p_states = states;
      }
   }

   virtual void elaborate_and_send_events() {
      const std::lock_guard<std::mutex> lock( _consignes_mutex );
      if( _en_panne_irréparable ) {
         event( event_t::PANNE_IRRÉPARABLE );
      }
      else {
         if( _porte_ouverte ) {
            event( event_t::PORTE_OUVERTE );
         }
         else {
            event( event_t::PORTE_FERMÉE );
         }
         if( _minuteur_ms < 1L ) {
            event( event_t::TEMPS_ÉCOULÉ );
         }
         else if( ! _porte_ouverte ) {
            event( event_t::MINUTEUR_POSITIF_ET_PORTE_FERMÉE );
         }
      }
   }

private:

   void allumer_la_lumière( void ) {
      _lumière_allumée = true;
   }

   void éteindre_la_lumière( void ) {
      _lumière_allumée = false;
   }

   void chauffer( void ) {
      std::cout << "Chauffer, _minuteur_ms = " << _minuteur_ms << std::endl;
      bool condition = false;
      {
         const std::lock_guard<std::mutex> lock( _consignes_mutex );
         condition = ( _minuteur_ms > 0 )&&( ! _porte_ouverte );
      }
      while( is_alive()&& condition ) {
         using namespace std::chrono_literals;
         std::this_thread::sleep_for( 10ms );
         const std::lock_guard<std::mutex> lock( _consignes_mutex );
         _minuteur_ms -= 10;
         condition = ( _minuteur_ms > 0 )&&( ! _porte_ouverte );
      }
   }

   void recycler( void ) {
      terminate();
   }

private:

   const std::string _in_path;
   const std::string _out_path;
   bool _porte_ouverte        = false;
   long _minuteur_ms          = 0L;
   bool _en_panne_irréparable = false;
   bool _lumière_allumée      = false;
   std::mutex _consignes_mutex;
};

int main( int argc, char * argv[] ) {
   const char * in_path  = "four à micro-ondes-in.txt";
   const char * out_path = "four à micro-ondes-out.txt";
   if( argc > 1 ) {
      in_path = argv[1];
      if( argc > 2 ) {
         out_path = argv[2];
      }
   }
   four_a_micro_onde four( in_path, out_path );
   four.run();
   return EXIT_SUCCESS;
}
