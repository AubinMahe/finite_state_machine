#pragma once
#include <string>

namespace famo {

   class entrées_sorties {
   public:

      entrées_sorties( void ) {}
      virtual ~ entrées_sorties( void ) {}

   // Entrées
   public:

      virtual bool obtenir(
         bool & la_porte_est_ouverte,
         long & la_valeur_du_minuteur_en_ms,
         bool & le_four_est_définitivement_en_panne ) = 0;

   // Sorties
   public:

      virtual bool publier(
         const std::string & le_nom_logique_de_l_état_de_l_automate,
         bool                la_lumière_est_allumée,
         bool                la_porte_est_ouverte,
         long                la_valeur_du_minuteur_en_ms,
         bool                le_four_est_définitivement_en_panne ) = 0;

   private:

      entrées_sorties( const entrées_sorties &) = delete;
      entrées_sorties & operator = ( const entrées_sorties &) = delete;
   };
}
