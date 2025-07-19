#include <hpms/fsm.hpp>
#include <famo/entrées_sorties.hpp>

namespace famo {

   enum class state_t {
      NONE = 0,
      ATTENDRE,
      CHAUFFER, // Rotation du plateau, émission des micro-ondes, décompte du temps
      HORS_D_USAGE,
   };

   enum class event_t {
      NONE = 0,
      PORTE_OUVERTE,
      PORTE_FERMÉE_ET_MINUTEUR_NUL,
      PORTE_FERMÉE_ET_MINUTEUR_POSITIF,
      MINUTEUR_MODIFIÉ,
      TEMPS_ÉCOULÉ,
      PANNE_IRRÉPARABLE
   };

   class four_à_micro_ondes :
      public hpms::fsm<state_t, event_t>
   {
   public:

      four_à_micro_ondes( famo::entrées_sorties & intrfc );

   protected:// Les entrées, événements et sorties

      virtual void acquire_sensors_statuses( void );
      virtual void elaborate_and_send_events( void );
      virtual void publish_actuators_commands( void );

   private:// Les actions

      bool allumer_la_lumière( void );
      bool éteindre_la_lumière( void );
      bool modifier_le_délai( void );
      bool recycler( void );

   private:// Les activités

      void chauffer( void );

   private:

      famo::entrées_sorties & _entrées_sorties;

      std::mutex _mutex;
      bool       _porte_ouverte        = false;
      long       _consigne_minuteur_ms = 0L;
      bool       _en_panne_irréparable = false;
      bool       _lumière_allumée      = false;
      long       _minuteur_ms          = 0L;
   };
}
