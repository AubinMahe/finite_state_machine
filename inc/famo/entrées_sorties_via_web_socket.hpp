#pragma once
#include <famo/entrées_sorties.hpp>

namespace famo {

   class entrées_sorties_via_web_socket : public entrées_sorties {
   public:

      entrées_sorties_via_web_socket( unsigned short port );

   // Entrées
   public:

      virtual bool obtenir(
         bool & la_porte_est_ouverte,
         long & la_valeur_du_minuteur_en_ms,
         bool & l_état_de_panne_définitive );

   // Sorties
   public:

      virtual void publier(
         const std::string & le_nom_logique_de_l_état_de_l_automate,
         bool la_lumière_est_allumée,
         bool la_porte_est_ouverte,
         long la_valeur_du_minuteur_en_ms,
         bool le_four_est_définitivement_en_panne );

   private:

      unsigned short _port;
   };
}
