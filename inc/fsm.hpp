#pragma once
#include <chrono>
#include <functional>
#include <map>
#include <thread>
#include <tuple>
#include <utility>

namespace hpms {

   template<typename S, typename E>
   class fsm {
   public:

      fsm( const S & initial, const std::initializer_list<std::tuple<S,E,S,std::function<void(void)>>> & transitions ) :
         _current( initial )
      {
         for( const auto & t : transitions ) {
            const S                         & from   = std::get<0>( t );
            const E                         & evt    = std::get<1>( t );
            const S                         & to     = std::get<2>( t );
            const std::function<void(void)> & action = std::get<3>( t );
            _transitions[std::pair<S,E>( from, evt )] = std::tuple<S,std::function<void(void)>>( to, action );
         }
      }

   protected:

      virtual void acquire_sensors_statuses( void ) = 0;
      virtual void elaborate_and_send_events( void ) = 0;

   public:

      void terminate( void ) {
         _dead = true;
      }

      bool is_alive() const {
         return ! _dead;
      }

      void run() {
         do {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for( 1ms );
            acquire_sensors_statuses();
            elaborate_and_send_events();
            if( _activity ) {
               std::thread( _activity ).detach();
               _activity = 0;
            }
         } while( is_alive());
      }

   protected:

      template<class F>
      void set_activity( const S & state, F && activity ) {
         _activities[state] = activity;
      }

      bool event( const E & event ) {
         auto iter = _transitions.find( std::pair<S,E>( _current, event ));
         if( iter == _transitions.end()) {
            // ignored event
            return false;
         }
         _current    = std::get<0>( iter->second );
         auto action = std::get<1>( iter->second );
         auto iter2  = _activities.find( _current );
         if( iter2 != _activities.end()) {
            _activity = iter2->second;
         }
         else {
            _activity = 0;
         }
         if( action ) {
            action();
         }
         return true;
      }
      
      S get_current_state() const {
         return _current;
      }

   private:

      bool _dead = false;
      std::function<void(void)> _activity = 0;
      S _current;
      std::map< std::pair< S, E >, std::tuple< S, std::function<void(void)>>> _transitions;
      std::map< S, std::function<void(void)>> _activities;
   };
}
