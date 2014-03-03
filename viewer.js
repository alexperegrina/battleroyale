
// *********************************************************************
// Global variables
// *********************************************************************

// Viewer state
var gamePaused = true;
var gamePreview = false; //If true, render will be called for the next tick even if game is paused, and then will be set to false
var gameDirection = 1;
var gameAnim = true;
var actRound = 0; //Current round index

// Data
var raw_data_str; //String for storing the raw data
var dataLoaded = false; //Set to true when raw_data_str is ready to be parsed
var data = { } //Object for storing all the game data

// Animation
var speed = 10; //Ticks per second
var FRAMES_PER_ROUND = 2;
var frames = 0; //Incremented each tick, when it reaches FRAMES_PER_ROUND, actRound is updated (acording to gameDirection)

// Visuals
var unitSize = 0.6; // 1 = same size as tile
var unitLineWidth = 2;
var grid_color = "#888888";
var cell_colors = {
    '0': "#FFAAAA",
    '1': "#AAFFAA",
    '2': "#0055FF",
    '3': "#AA88FF",
    'X': grid_color,
    '.': "#111111"
}
var player_colors = {
    0: "#FF0000",
    1: "#008800",
    2: "#00FFFF",
    3: "#9900CC",
}




// *********************************************************************
// Utility functions
// *********************************************************************


function getURLParameter (name) {
    // http://stackoverflow.com/questions/1403888/get-url-parameter-with-jquery
    var a = (RegExp(name + '=' + '(.+?)(&|$)').exec(location.search)||[,null])[1]
    if (a != null) return decodeURI(a);
    return null;
}

//Callback has a single parameter with the file contents
function loadFile (file, callback) {
    
    var xmlhttp;
    
    if (file == null || file == "") {
        alert("You must specify a file to load");
        return;
    }

    if (window.XMLHttpRequest) {
        // code for IE7+, Firefox, Chrome, Opera, Safari
        xmlhttp = new XMLHttpRequest();
    } else {
        // code for IE6, IE5
        xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
    }
    
    // http://www.w3schools.com/ajax/ajax_xmlhttprequest_onreadystatechange.asp
    xmlhttp.onreadystatechange = function() {
        if (xmlhttp.readyState == 4) {
            //Note: We can not check xmlhttp.status != 200 for errors because status is not set when loading local files
            callback(xmlhttp.responseText);
        }
    }

    xmlhttp.open("GET", file, false);
    xmlhttp.send();
}

function int (s) {
    return parseInt(s);
}

function double (s) {
    return parseFloat(s);
}




// *********************************************************************
// Initialization functions
// *********************************************************************

function parseData (raw_data_str) {
    
    if ("" == raw_data_str) {
        alert("Could not load game file");
        return false;
    }
    
    // convert text to tokens
    var st = raw_data_str + "";
    var t = st.replace('\n', ' ').split(/\s+/);
    var p = 0;

    // read prelude

    s = t[p++];
    data.version = t[p++];
    if (s != "battleroyale") {
		//alert("Error parsing file, expected token: " + "battleroyale");
		alert("Are you sure this is a Battle Royale game file?");
		document.getElementById('file').value = "";
		document.getElementById('inputdiv').style.display = "";
		document.getElementById('loadingdiv').style.display = "none";
		return false;
	}
    if (data.version != "v1") alert("Error parsing file, expected token: " + "version");

    s = t[p++];
    data.nb_players = int(t[p++]);
    if (s != "nb_players") alert("Error parsing file, expected token: " + "nb_players");

    s = t[p++];
    data.nb_rounds = int(t[p++]);
    if (s != "nb_rounds") alert("Error parsing file, expected token: " + "nb_rounds");

    s = t[p++];
    data.nb_farmers = int(t[p++]);
    if (s != "nb_farmers") alert("Error parsing file, expected token: " + "nb_farmers");

    s = t[p++];
    data.nb_knights = int(t[p++]);
    if (s != "nb_knights") alert("Error parsing file, expected token: " + "nb_knights");

    s = t[p++];
    data.farmers_health = int(t[p++]);
    if (s != "farmers_health") alert("Error parsing file, expected token: " + "farmers_health");

    s = t[p++];
    data.knights_health = int(t[p++]);
    if (s != "knights_health") alert("Error parsing file, expected token: " + "knights_health");

    s = t[p++];
    data.farmers_regen = int(t[p++]);
    if (s != "farmers_regen") alert("Error parsing file, expected token: " + "farmers_regen");

    s = t[p++];
    data.knights_regen = int(t[p++]);
    if (s != "knights_regen") alert("Error parsing file, expected token: " + "knights_regen");

    s = t[p++];
    data.damage_min = int(t[p++]);
    if (s != "damage_min") alert("Error parsing file, expected token: " + "damage_min");

    s = t[p++];
    data.damage_max = int(t[p++]);
    if (s != "damage_max") alert("Error parsing file, expected token: " + "damage_max");

    s = t[p++];
    data.rows = int(t[p++]);
    if (s != "rows") alert("Error parsing file, expected token: " + "rows");

    s = t[p++];
    data.cols = int(t[p++]);
    if (s != "cols") alert("Error parsing file, expected token: " + "cols");

    data.nb_units = data.nb_players * (data.nb_knights + data.nb_farmers);

    s = t[p++];
    if (s != "names") alert("Error parsing file, expected token: " + "names");
    data.names = new Array();
    for (var i = 0; i < data.nb_players; ++i) {
        data.names[i] = t[p++];
    }

    data.rounds = new Array();
    for (var round = 0; round < data.nb_rounds; ++round) {

        if (t[p++] != "round") alert("Error parsing file, expected token: " + "round");
        if (int(t[p++]) != round) alert("Error parsing file, expected token: " + "wrong round");

        // maze
        data.rounds[round] = new Object();
        data.rounds[round].rows = new Array();
        for (var i = 0; i < data.rows; ++i) {
            data.rounds[round].rows[i] = t[p++];
        }
        
        // score
        if (t[p++] != "score") alert("Error parsing file, expected token: " + "score");
        data.rounds[round].score = new Array();
        for (var i = 0; i < data.nb_players; ++i) {
            data.rounds[round].score[i] = int(t[p++]);
        }

        // status
        if (t[p++] != "status") alert("Error parsing file, expected token: " + "status");
        data.rounds[round].cpu = new Array();
        for (var i = 0; i < data.nb_players; ++i) {
            data.rounds[round].cpu[i] = double(t[p++]);
        }

        //units
        data.rounds[round].units = [ ];
        for (var i = 0; i < data.nb_units; ++i) {
            var unit = {
                type:   t[p++],
                player: int(t[p++]),
                i:      int(t[p++]),
                j:      int(t[p++]), 
                health: int(t[p++]),
                move: '' //Will be set when reading movements
            };
            data.rounds[round].units.push(unit);
        }

        if (round != data.nb_rounds - 1) {
            // actions
            if (t[p++] != "actions") alert("Error parsing file, expected token: " + "no actions");
            for (var player = 0; player < data.nb_players; ++player) {
                if (int(t[p++]) != player) alert("Error parsing file, expected token: " + "wrong player");
                
                var code = int(t[p++]);
                while (code != -1) {
                    var action = {
                        id: code,
                        dir: t[p++]
                    };
                    //We don't need to store the actions so I'm skipping them
                    //data.rounds[round].players[player].actions.push(action);
                    code = int(t[p++]);
                }
                
            }

            // movements
            if (t[p++] != "movements") alert("Error parsing file, expected token: " + "no movements");

            var code = int(t[p++]);
            while (code != -1) {
                data.rounds[round].units[code].move = t[p++];
                code = int(t[p++]);
            }
            
        }

    }
	
	return true;
	
}

//Initializing the game
function initGame (raw_data) {
   
	document.getElementById("loadingdiv").style.display="";
	
    //TODO: Next two calls could run concurrently
    if (parseData(raw_data) === false) return;
    preloadImages();
	
    // prepare state variables
    /*if (getURLParameter("start") == "yes") gamePaused = false;
    else gamePaused = true;*/
	gamePaused = false;
	
    gamePreview = true;

    // slider init
    $("#slider").slider( "option", "min", 0 );
    $("#slider").slider( "option", "max", data.nb_rounds );
        
    // Canvas element
    canvas = document.getElementById('myCanvas');
    context = canvas.getContext("2d");
    
    // prepare the slider
    $("#slider").slider({
        slide: function(event, ui) {
            var value = $("#slider").slider( "option", "value" );
            actRound = value;
            frames = 0;
            gamePaused = true;
            gamePreview = true;
        }
    });
    $("#slider").width(500);
	    
    // set the listerners for interaction
    document.addEventListener('mousewheel', onDocumentMouseWheel, false);
    document.addEventListener('keydown', onDocumentKeyDown, false);
    document.addEventListener('keyup', onDocumentKeyUp, false);

    window.addEventListener('resize', onWindowResize, false);
    onWindowResize();
    	
	document.getElementById("loadingdiv").style.display="none";
	document.getElementById("gamediv").style.display="";
		
    mainloop();
}


function preloadImages () {

    data.img = new Array();

    //Background
    //data.img.background = new Image();
    //data.img.background.src = "img/tile_floor.png";

}




// *********************************************************************
// Main loop functions
// *********************************************************************

function updateGame () {
    /*
    if (actRound >= 0 && actRound < data.nb_rounds) {
        for (var i = 0; i < data.nb_players; ++i) {
            var f = (frames+i)%4;
            switch (data.rounds[actRound].team[i].pacman.action) {
                case 't': //Top
                    data.img.spr_pacman[i] = data.img.spr_pacman_t[i][f];
                    break;
                case 'b': //Bottom
                    data.img.spr_pacman[i] = data.img.spr_pacman_b[i][f];
                    break;
                case 'r': //Right
                    data.img.spr_pacman[i] = data.img.spr_pacman_r[i][f];
                    break;
                case 'l': //Left
                    data.img.spr_pacman[i] = data.img.spr_pacman_l[i][f];
                    break;
                default: //None
                    data.img.spr_pacman[i] = data.img.spr_pacman_r[i][f];
                    break;
    }   }   }
    */
    $("#slider").slider("option", "value", actRound);
}


function writeGameState () {
    // write round
    $("#round").html("Round: " + actRound);

    //update scoreboard
    var scoreboard = "";
    for (var i = 0; i < data.nb_players; i++) {
        scoreboard += "<span class='score'>"
        
        + "<div style='display:inline-block; margin-top: 5px; width:20px; height:20px; background-color:"+ cell_colors[i] +"'></div>"
        + "<div style='display:inline-block; vertical-align: middle; margin-bottom: 10px; margin-left:5px;'>"+data.names[i]+": " + data.rounds[actRound].score[i]+"</div>"
        
        + "</span>";
    }
    $("#scores").html(scoreboard);
}

function drawGame () {
   
    //Boundary check
    if (actRound < 0) actRound = 0;
    if (actRound >= data.nb_rounds) actRound = data.nb_rounds-1;
    
    //Outter Rectangle
    context.fillStyle = grid_color;
    context.fillRect(0, 0, tileSize*data.cols, tileSize*data.rows);
    //Draw maze
    var rows = data.rounds[actRound].rows;
    for (var i = 0; i < data.rows; i++) {
        var row = rows[i];
        for (var j = 0; j < data.cols; ++j) {
            var cell = row[j];
            context.fillStyle = cell_colors[cell];
            context.fillRect(j*tileSize, i*tileSize, tileSize-0.5, tileSize-0.5); //-1 to show a grid
        }
    }

    //Draw units
    context.lineWidth = unitLineWidth;
    
    var units = data.rounds[actRound].units;
    for (var un in units) {
        var u = units[un];
    
        context.strokeStyle = player_colors[u.player];

        var i = u.i;
        var j = u.j;
            
        if (gameAnim) {
            //TODO: Currar-se mes la animacio
            if (frames >= FRAMES_PER_ROUND/2) {
                if (u.move == 't') i -= 0.5;
                else if (u.move == 'b') i += 0.5;
                else if (u.move == 'r') j += 0.5;
                else if (u.move == 'l') j -= 0.5;
            }
        }

        if (u.type == 'k') drawKnight(i,j);
        else drawFarmer(i,j);
       
    }
}

function drawKnight (i,j) {
    var size = unitSize * tileSize;
    var offset = (tileSize - size) / 2;
    context.strokeRect(j*tileSize + offset, i*tileSize + offset, size, size);
}

function drawFarmer (i,j) {
    var size = unitSize * tileSize;
    var offset = (tileSize - size) / 2;
    context.beginPath();
    context.arc(j*tileSize + size/2 + offset, i*tileSize + size/2 + offset, size/2, 0, Math.PI*2); 
    context.closePath();
    context.stroke();
}




// *********************************************************************
// Button events
// *********************************************************************

function playButton () {
    if (actRound >= data.nb_rounds - 1) actRound = 0;
    gamePaused = false;
}

function pauseButton () {
    gamePaused = true;
    gamePreview = true; //To call render again
    frames = 0;
}

function startButton () {
    actRound = 0;
    frames = 0;
    gamePreview = true;
    gamePaused = true;
}

function endButton () {
    actRound = data.nb_rounds-1;
    frames = 0;
    gamePreview = true;
}

function animButton () {
    gameAnim = !gameAnim;
}

function closeButton () {
    window.close();
}





// *********************************************************************
// Keyboard and Mouse events
// *********************************************************************

function onDocumentMouseWheel (event) {
}

function onDocumentKeyDown (event) {
}

function onDocumentKeyUp (event) {

    // http://www.webonweboff.com/tips/js/event_key_codes.aspx

    switch (event.keyCode) {

        case 36: // Start
            actRound = 0;
            frames = 0;
            gamePreview = true;
            break;

        case 35: // End
            actRound = data.nb_rounds-1;
            frames = 0;
            gamePreview = true;
            break;

        case 33: // PageDown
            actRound -= 10;
            frames = 0;
            gamePreview = true;
            break;

        case 34: // PageUp
            actRound += 10;
            frames = 0;
            gamePreview = true;
            break;

        case 38: // ArrowUp
        case 37: // ArrowLeft
            gamePaused= true;
            frames = 0;
            --actRound;
            gamePreview = true;
            break;

        case 40: // ArrowDown
        case 39: // ArrowRight
            gamePaused = true;
            frames = 0;
            ++actRound;
            gamePreview = true;
            break;

        case 32: // Space
            if (gamePaused) playButton();
            else pauseButton();
            break;

        case 72: // "h"
            help();
            break;

        default:
            //$("#debug").html(event.keyCode);
            break;
    }
}




function onWindowResize (event) {
    
    //Constants
    var header_height = 130;
    var canvas_margin = 20;
    
    // set canvas size
    var size = Math.min(document.body.offsetWidth, document.body.offsetHeight - header_height) - canvas_margin*2;
    
    canvas.width  = size;
    canvas.height = size;
    
    var max_dimension = Math.max(data.cols,data.rows);
    tileSize = size / max_dimension;
          
    drawGame();

}


function help () {
    // opens a new popup with the help page
    var win = window.open('help.html' , 'name', 'height=400, width=300');
    if (window.focus) win.focus();
    return false;
}



// *********************************************************************
// This function is called periodically.
// *********************************************************************

function mainloop () {
    
    // Configure buttons
    if (gamePaused) {
        $("#but_play").show();
        $("#but_pause").hide();
    } else {
        $("#but_play").hide();
        $("#but_pause").show();
    }

    if (actRound < 0) actRound = 0;
    if (actRound >= data.nb_rounds) {
        actRound = data.nb_rounds;
        gamePaused = true;
        frames = 0;
    }

    if (!gamePaused || gamePreview) {

        updateGame();
        drawGame();
        writeGameState();

        if (gamePreview) {
            frames = 0;
            gamePreview = false;
        } else {
            frames++;
            if (frames == FRAMES_PER_ROUND) {
                actRound += gameDirection;
                frames = 0;
            }
        }
                
    }
    // periodically call mainloop
    var frame_time = 1000/speed;
    setTimeout(mainloop, frame_time);
}





// *********************************************************************
// Main function, it is called when the document is ready.
// *********************************************************************

function init () {

    // get url parameters
    var game;
    if (getURLParameter("sub") != null) {
        if (getURLParameter("nbr") != null) {
            game = "https://battle-royale.jutge.org/?cmd=lliuraments&sub="+getURLParameter("sub")+"&nbr="+getURLParameter("nbr")+"&download=partida";
        } else {
            game = "https://battle-royale.jutge.org/?cmd=partida&sub="+getURLParameter("sub")+"&download=partida";
        }
    } else {
        game = getURLParameter("game");
    }
	
	if (game == null || game == "") {
		// ask the user for a game input
		var inputdiv = document.getElementById('inputdiv')
		inputdiv.style.display = "";
		document.getElementById('file').addEventListener('change', function(evt) {

			//http://www.html5rocks.com/en/tutorials/file/dndfiles/
			var file = evt.target.files[0];
			var reader = new FileReader();
			reader.readAsText(file);
			reader.onloadend = function(evt) {
				if (evt.target.readyState == FileReader.DONE) { // DONE == 2
					inputdiv.style.display = "none";
					document.getElementById("loadingdiv").style.display="";
					initGame(reader.result);
				} else {
					alert("Error accessing file");
				}
			};
			
		}, false);
	} else {
		document.getElementById("loadingdiv").style.display="";
		// load the given game
		loadFile(game, initGame);
	}
	
}
