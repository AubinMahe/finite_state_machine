#pragma once
#include <string>

namespace famo {

   class entrées_sorties {
   // Entrées
   public:

      virtual bool obtenir(
         bool & la_porte_est_ouverte,
         long & la_valeur_du_minuteur_en_ms,
         bool & l_état_de_panne_définitive ) = 0;

   // Sorties
   public:

      virtual void publier(
         const std::string & le_nom_logique_de_l_état_de_l_automate,
         bool la_lumière_est_allumée,
         bool la_porte_est_ouverte,
         long la_valeur_du_minuteur_en_ms,
         bool le_four_est_définitivement_en_panne ) = 0;
   };
}
