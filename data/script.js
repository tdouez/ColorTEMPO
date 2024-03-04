/* sav config */
function savButton() {
    console.log("Sav config");
    console.log($("#form_config").serialize());
    $.post('/config.htm', $("#form_config").serialize())
        .done( function(msg, textStatus, xhr) {
            console.log($("#form_config").serialize());
            $.notify("Enregistrement effectué", "success");
        })
        .fail( function(xhr, textStatus, errorThrown) {
            $.notify("Enregistrement impossible "+ errorThrown, "error");
        })
        console.log("Form Submit config");
}

/* fonction lecture json */
function lire_item_json_info(myjson) {
    console.log("lire item json info");


    let j = JSON.parse(myjson, function(name, value) {

        if (name == "version") {
            document.getElementById("version").innerHTML = value;
        }
        if (name == "module_name") {
            document.getElementById("module_name").innerHTML = value;
        }
		    if (name == "date") {
            document.getElementById("date").innerHTML = value;
        }
        if (name == "couleur_jour") {
            document.getElementById("couleur_jour").innerHTML = value;
			      document.getElementById("couleur_jour").style.color = 'orange';
			      if (value.indexOf("BLEU") != -1) document.getElementById("couleur_jour").style.color = 'blue';
			      if (value.indexOf("BLANC") != -1) document.getElementById("couleur_jour").style.color = 'black';
			      if (value.indexOf("ROUGE") != -1) document.getElementById("couleur_jour").style.color = 'red';
        }
		    if (name == "couleur_demain") {
            document.getElementById("couleur_demain").innerHTML = value;
			      document.getElementById("couleur_demain").style.color = 'orange';
			      if (value.indexOf("BLEU") != -1) document.getElementById("couleur_demain").style.color = 'blue';
			      if (value.indexOf("BLANC") != -1) document.getElementById("couleur_demain").style.color = 'black';
			      if (value.indexOf("ROUGE") != -1) document.getElementById("couleur_demain").style.color = 'red';
        }
		    if (name == "heure") {
            if (value == "HP") {
				          document.getElementById("heure").style.color = 'red';
				          document.getElementById("heure").innerHTML = value;
			      }
			      if (value == "HC") {
				          document.getElementById("heure").style.color = 'green';
				          document.getElementById("heure").innerHTML = value;
			      }
        }
		    if (name == "etat_relais") {
            document.getElementById("etat_relais").innerHTML = value;
        }
        if (name == "info") {
            affiche_info_info(value);
        }
        if (name == "erreur_info") {
            affiche_erreur_info(value);
        }
        if (name == "current_date") {
            document.getElementById("currentDate").innerHTML = value;
        }

    });
}

/* fonction lecture json */
function lire_item_json_config(myjson) {
    console.log("lire item json cfg");

    document.getElementById("b_hc_name").checked = false;
    document.getElementById("b_hp_name").checked = false;
    document.getElementById("w_hc_name").checked = false;
    document.getElementById("w_hp_name").checked = false;
    document.getElementById("r_hc_name").checked = false;
    document.getElementById("r_hp_name").checked = false;

    let j = JSON.parse(myjson, function(name, value) {

        if (name == "b_hc_name") {
          if (value == 1) document.getElementById("b_hc_name").checked = true;
        }
        if (name == "b_hp_name") {
          if (value == 1) document.getElementById("b_hp_name").checked = true;
        }
        if (name == "w_hc_name") {
          if (value == 1) document.getElementById("w_hc_name").checked = true;
        }
        if (name == "w_hp_name") {
          if (value == 1) document.getElementById("w_hp_name").checked = true;
        }
        if (name == "r_hc_name") {
          if (value == 1) document.getElementById("r_hc_name").checked = true;
        }
        if (name == "r_hp_name") {
          if (value == 1) document.getElementById("r_hp_name").checked = true;
        }

    });
}


/* affiche info sur panel info*/
function affiche_info_info(buffer) {
    document.getElementById("t_info").innerHTML = buffer;
    if (buffer == "") {
        document.getElementById("p_info").style.display = "none";
    }
    else {
        document.getElementById("p_info").style.display = "block";
    }
}

/* affiche erreur sur panel info*/
function affiche_erreur_info(buffer) {
    document.getElementById("t_erreur_info").innerHTML = buffer;
    if (buffer == "") {
        document.getElementById("p_erreur_info").style.display = "none";
    }
    else {
        document.getElementById("p_erreur_info").style.display = "block";
    }
}


/* mise a jour info module */
function maj_infos() {
    console.log("maj infos");

    $.getJSON("/info.json", function(data) {
        let myNewJSON = JSON.stringify(data, null, '\t');
        lire_item_json_info(myNewJSON);
    })
    .fail(function(xhr, textStatus, errorThrown) {
        console.log( "error " + errorThrown );
    });
}

/* init */
function init () {
    console.log("get /config.json");

    $.getJSON("/config.json", function(data) {
        let myNewJSON = JSON.stringify(data, null, '\t');
        lire_item_json_config(myNewJSON);
    })
    .fail(function(xhr, textStatus, errorThrown) {
        console.log( "error " + errorThrown );
    });
    $.getJSON("/info.json", function(data) {
        let myNewJSON = JSON.stringify(data, null, '\t');
        lire_item_json_info(myNewJSON);
    })
    .fail(function(xhr, textStatus, errorThrown) {
        console.log( "error " + errorThrown );
    });
}

window.onload=init();
timer = window.setInterval("maj_infos()", 5000);
