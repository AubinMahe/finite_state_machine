const fsm_service = new WebSocket("ws://127.0.0.1:2416",[ "fsm" ]);
const ui_state    = document.getElementById("state");
const ui_porte    = document.getElementById("porte");
const ui_lumière  = document.getElementById("lumière");
const ui_minuteur = document.getElementById("minuteur");
const four        = {
   état_courant                       : "",
   la_porte_est_ouverte               : false,
   la_lumière_est_allumée             : false,
   la_valeur_du_minuteur_en_ms        : 0,
   le_four_est_définitivement_en_panne: false
};

fsm_service.onopen = function () {
   console.log("Connected to WebSocket!");
};

fsm_service.onmessage = function (event) {
   console.log("fsm_service.onmessage|data: '" + event.data + "'");
   const msg = JSON.parse( event.data );
   ui_state  .value   = msg.état_courant;
   // ui_porte  .checked = msg.la_porte_est_ouverte;
   ui_lumière.checked = msg.la_lumière_est_allumée;
   four.la_valeur_du_minuteur_en_ms = msg.la_valeur_du_minuteur_en_ms;
   if( msg.le_four_est_définitivement_en_panne ) {
      fsm_service.close();
      fsm_service = null;
      window.close();
   }
};

fsm_service.onclose = function () {
   console.log("Connection closed");
};

fsm_service.onerror = function (error) {
   console.log("Error: " + error);
};

function onPorteChange() {
   four.la_porte_est_ouverte = ui_porte.checked;
   fsm_service.send( JSON.stringify( four ));
}

function onMinuteurChange() {
   four.la_valeur_du_minuteur_en_ms = 1000 * parseInt( ui_minuteur.value );
   fsm_service.send( JSON.stringify( four ));
}

function onPanneClicked() {
   four.le_four_est_définitivement_en_panne = true;
   fsm_service.send( JSON.stringify( four ));
}

document.getElementById("panne").addEventListener('click', e => onPanneClicked());
ui_porte   .addEventListener('change', e => onPorteChange());
ui_minuteur.addEventListener('change', e => onMinuteurChange());
