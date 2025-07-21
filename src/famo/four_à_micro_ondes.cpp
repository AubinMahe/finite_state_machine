#include <hpms/fsm.hpp>
#include <famo/four_à_micro_ondes.hpp>

#include <chrono>
#include <iostream>

namespace famo {

   std::ostream & operator << ( std::ostream & stream, const state_t & state ) {
      switch( state ) {
         case state_t::NONE        : return stream << "NONE";
         case state_t::ATTENDRE    : return stream << "ATTENDRE";
         case state_t::CHAUFFER    : return stream << "CHAUFFER";
         case state_t::HORS_D_USAGE: return stream << "HORS_D_USAGE";
      }
      return stream << "???";
   }

   std::ostream & operator << ( std::ostream & stream, const event_t & event ) {
      switch( event ) {
         case event_t::NONE                            : return stream << "NONE";
         case event_t::PORTE_OUVERTE                   : return stream << "PORTE_OUVERTE";
         case event_t::PORTE_FERMÉE_ET_MINUTEUR_NUL    : return stream << "PORTE_FERMÉE_ET_MINUTEUR_NUL";
         case event_t::PORTE_FERMÉE_ET_MINUTEUR_POSITIF: return stream << "PORTE_FERMÉE_ET_MINUTEUR_POSITIF";
         case event_t::MINUTEUR_MODIFIÉ                : return stream << "MINUTEUR_MODIFIÉ";
         case event_t::TEMPS_ÉCOULÉ                    : return stream << "TEMPS_ÉCOULÉ";
         case event_t::PANNE_IRRÉPARABLE               : return stream << "PANNE_IRRÉPARABLE";
      }
      return stream << "???";
   }
}

using namespace famo;

/**
 * Légende
 *=========
 * ╔═══════════════════╗
 * ║ ACTIVITÉ INITIALE ║
 * ╚═══════════════════╝
 * ┌──────────┐
 * │ ACTIVITÉ │ Une activité prend du temps, elle dure.
 * └──────────┘
 * (événement)  Un événement est généré par consultation de l'état du système,
 *              ce dernier pouvant être modifié par une action extérieure (humaine en l'occurence).
 * [action]     Une action est ponctuelle, sa durée est négligeable.
 *
 * Automate d'états
 *==================
 *    ╔══════════╗
 * ┌─>║ ATTENDRE ╟─>──────────────────────────────>─────────────────────┬─>─┬─>────────────────>───────────────>─┐
 * ↑  ╚╤═══╤═══╤═╝                                                      │   ↓                                    ↓
 * │   ↓   ↑   ↑ ((Minuteur = 0) et (Porte fermée)[Éteindre la lumière] │   │                                    │
 * │   │   │   └───────────────<──────────────────<───────────────────<─┘   │                                    │
 * │   │   │                                                                │                                    │
 * │   │   │        (Porte ouverte)[Allumer la lumière]                     ↓                                    │
 * │   │   └─────────────────────<────────────────────────────<───────┬───<─┘                                    │
 * │   │                                                              ↑                                          ↓
 * │   ↓ ((Minuteur > 0) et (Porte fermée))[Allumer la lumière]   ┌───┴──────┐                                   │
 * │   └────────────────>─────────────────>──────────────────────>│ CHAUFFER ├─>──────────────>──────────────>─┐ │
 * │                                                              └─┬──┬──┬──┘                                 │ │
 * ↑                            (Temps écoulé)[Éteindre la lumière] ↓  ↓  ↑ (Minuteur modifié)[Modifier délai] │ │
 * └────────────<─────────────────────────────<───────────────────<─┘  │  └───────────────────<──────────────<─┘ │
 *                                                                     │                                         │
 *    ┌──────────────┐                (En panne irréparable)[Recycler] ↓                                         ↓
 *    │ HORS D'USAGE ├─<───────────<─────────────────────<─────────────┴────<──────────────────────────────────<─┘
 *    └──────────────┘
 *
 * <https://cloford.com/resources/charcodes/utf-8_box-drawing.htm>
*/
four_à_micro_ondes::four_à_micro_ondes( entrées_sorties & es, bool verbose ) :
   hpms::fsm<state_t, event_t>(
      state_t::ATTENDRE, {
         {state_t::ATTENDRE, event_t::PORTE_OUVERTE                   , state_t::ATTENDRE    , [this](){ return allumer_la_lumière();  }},
         {state_t::ATTENDRE, event_t::PORTE_FERMÉE_ET_MINUTEUR_NUL    , state_t::ATTENDRE    , [this](){ return éteindre_la_lumière(); }},
         {state_t::ATTENDRE, event_t::PORTE_FERMÉE_ET_MINUTEUR_POSITIF, state_t::CHAUFFER    , [this](){ return allumer_la_lumière() && modifier_le_délai();  }},
         {state_t::ATTENDRE, event_t::PANNE_IRRÉPARABLE               , state_t::HORS_D_USAGE, [this](){ return recycler();            }},
         {state_t::CHAUFFER, event_t::MINUTEUR_MODIFIÉ                , state_t::CHAUFFER    , [this](){ return modifier_le_délai();   }},
         {state_t::CHAUFFER, event_t::TEMPS_ÉCOULÉ                    , state_t::ATTENDRE    , [this](){ return éteindre_la_lumière(); }},
         {state_t::CHAUFFER, event_t::PORTE_OUVERTE                   , state_t::ATTENDRE    , [this](){ return allumer_la_lumière();  }},
         {state_t::CHAUFFER, event_t::PANNE_IRRÉPARABLE               , state_t::HORS_D_USAGE, [this](){ return recycler();            }},
      },
      verbose
   ),
   _entrées_sorties( es )
{
   set_activity( state_t::CHAUFFER, [this](){ chauffer(); });
}

void four_à_micro_ondes::acquire_sensors_statuses( void ) {
   const std::lock_guard<std::mutex> lock( _mutex );
   _entrées_sorties.obtenir(
      _porte_ouverte,
      _consigne_minuteur_ms,
      _en_panne_irréparable );
}

void four_à_micro_ondes::elaborate_and_send_events( void ) {
   const std::lock_guard<std::mutex> lock( _mutex );
   if( _en_panne_irréparable ) {
      event( event_t::PANNE_IRRÉPARABLE );
   }
   else if( _porte_ouverte ) {
      event( event_t::PORTE_OUVERTE );
   }
   else if( _consigne_minuteur_ms == 0L ) {
      event( event_t::PORTE_FERMÉE_ET_MINUTEUR_NUL );
   }
   else {
      event( event_t::PORTE_FERMÉE_ET_MINUTEUR_POSITIF );
   }
}

void four_à_micro_ondes::publish_actuators_commands( void ) {
   const std::lock_guard<std::mutex> lock( _mutex );
   std::stringstream ss;
   ss << get_current_state();
   _entrées_sorties.publier(
      ss.str(),
      _lumière_allumée,
      _porte_ouverte,
      _minuteur_ms,
      _en_panne_irréparable );
}

bool four_à_micro_ondes::allumer_la_lumière( void ) {
   _lumière_allumée = true;
   return true;
}

bool four_à_micro_ondes::éteindre_la_lumière( void ) {
   _lumière_allumée = false;
   return true;
}

bool four_à_micro_ondes::modifier_le_délai( void ) {
   _minuteur_ms = _consigne_minuteur_ms;
   return true;
}

bool four_à_micro_ondes::recycler( void ) {
   terminate();
   return true;
}

void four_à_micro_ondes::chauffer( void ) {
   bool condition = false;
   {
      const std::lock_guard<std::mutex> lock( _mutex );
      condition = ( _minuteur_ms > 0 )&&( ! _porte_ouverte );
   }
   while( is_alive()&& condition ) {
      using namespace std::chrono_literals;
      std::this_thread::sleep_for( 20ms );
      const std::lock_guard<std::mutex> lock( _mutex );
      if( _minuteur_ms != _consigne_minuteur_ms ) {
         event( event_t::MINUTEUR_MODIFIÉ );
      }
      _minuteur_ms -= 20;
      condition = ( _minuteur_ms > 0 )&&( ! _porte_ouverte );
      _consigne_minuteur_ms = _minuteur_ms;
   }
   if( _minuteur_ms <= 0L ) {
      event( event_t::TEMPS_ÉCOULÉ );
   }
}
