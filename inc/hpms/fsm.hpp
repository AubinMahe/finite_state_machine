#pragma once
#include <chrono>
#include <functional>
// #include <iostream>
#include <map>
#include <thread>
#include <tuple>
#include <utility>

namespace hpms {

   template<typename S, typename E>
   class fsm {
   public:

      typedef std::function<bool(void)> action_t;
      typedef std::function<void(void)> activity_t;

      fsm( S initial, const std::initializer_list<std::tuple<S,E,S,action_t>> & transitions ) :
         _current( initial )
      {
         for( auto t : transitions ) {
            S        from   = std::get<0>( t );
            E        evt    = std::get<1>( t );
            S        to     = std::get<2>( t );
            action_t action = std::get<3>( t );
            _transitions[std::pair<S,E>( from, evt )] = std::pair<S,action_t>( to, action );
         }
      }

   protected:

      virtual void acquire_sensors_statuses( void ) = 0;
      virtual void elaborate_and_send_events( void ) = 0;
      virtual void publish_actuators_commands( void ) = 0;

   public:

      void terminate( void ) {
         _dead = true;
      }

      bool is_alive() const {
         return ! _dead;
      }

      /**
       * Tâche de période 20 ms.
       */
      void run() {
         do {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for( 20ms );
            acquire_sensors_statuses();
            elaborate_and_send_events();
            publish_actuators_commands();
         } while( is_alive());
      }

   protected:

      template<class F>
      void set_activity( S state, F && activity ) {
         _activities[state] = activity;
      }

      bool event( E event ) {
         auto iter = _transitions.find( std::pair<S,E>( _current, event ));
         if( iter == _transitions.end()) {
            // ignored event
            return false;
         }
         action_t action = iter->second.second;
         bool     tr_ok  = true;
         if( action ) {
            tr_ok = action();
         }
         // action() must be successful to change state.
         if( tr_ok ) {
            S prev = _current;
            _current = iter->second.first;
            if( _current != prev ) {
               auto iter2 = _activities.find( _current );
               if( iter2 != _activities.end()) {
                  // const auto now = std::chrono::system_clock::now();
                  // const std::time_t t_c = std::chrono::system_clock::to_time_t( now );
                  // std::cout << std::ctime(&t_c) << " - std::thread\n";
                  std::thread( iter2->second ).detach();
               }
            }
         }
         return true;
      }
      
      S get_current_state() const {
         return _current;
      }

   private:

      std::map<
         std::pair<S,E>,
         std::pair<S,action_t>> _transitions;
      std::map<S,activity_t>    _activities;
      bool                      _dead = false;
      S                         _current;
   };
}
