//v2: Send to just 1 IP the # of pixel to color.
//V1: Color matrix and wobsocket project for 64 objects
//Code taken from schwimmende pixel and adapted to laufpixelproject.

//Hier sind die Funktionen für die Webseite
//Hier ist auch der Websocketteil implementiert


//var websocket= [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
 //           ,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0];  //websocket-"Object"
//var outputDataStream = "";  //message, will be sent, if websocket-server is idle
//var connectedToServer //flag, shows if client is connected to server (set by onOpen, reset by onClose)
//var serverIdle = false; //flag, shows if server is idle. Is set when server sends "--serverIdle--"

var websocket = [0,0];  //websocket arry mit NULL inizialisiert
var color= [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
            ,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0];       //Array für das Merken welche Farbe in welchem Button
//var ip1 = "ws://192.168.1.169/ws";    //Wenn nur ein ESP
var ip ={1:"ws://192.168.188.166/ws"}//"ws://192.168.101.220/ws" "ws://192.168.0.32/ws" "ws://192.168.188.166/ws"
var dataSent = [0,0];
    
// Funktion for the LED_Buttons

function wechselfarbe(clicked_id) {
    color[clicked_id]++;
    if (color[clicked_id] > 3){
        color[clicked_id] = 1;
    }
    switch(color[clicked_id]){
        case 1:
            document.getElementById(clicked_id).style.backgroundColor = "red";
            dataSent [0] = color[clicked_id];
			dataSent[1] =clicked_id;
			websocket[1].send(dataSent);
            break;
        case 2:
            document.getElementById(clicked_id).style.backgroundColor = "green";
            dataSent [0] = color[clicked_id];
			dataSent[1] =clicked_id;
			websocket[1].send(dataSent);
            break;
        case 3:
            document.getElementById(clicked_id).style.backgroundColor = "blue";
            dataSent [0] = color[clicked_id];
			dataSent[1] =clicked_id;
			websocket[1].send(dataSent);
            break;
    }
};

// Reset_Button
function rest(){
    for (var i = 1; i < 10; i++){
    //document.getElementById(i).style.backgroundColor = "black";
    //color[i]= 0;
    }
}


// Function to Connect to all ESP32
function connect() {
    //for (var i = 1; i < 9; i++){
        mainWebSocket(ip[1],1);
    //}
}


function mainWebSocket(wsUri,id) {
    websocket[id] = new WebSocket(ip[id]);   //creating new WebSocket-Object, constructor needs IP-Adress (here...)
    websocket[id].onopen = function () { onOpen(id); }; //function called by websocket.onopen-method (when websocket-connection is started)
    websocket[id].onclose = function () { onClose(); };   //function called by websocket.onclose-method (when websocket-connection is closed)
    //websocket[id].onmessage = function (evt) { onMessage(evt); };   //function called by websocket.onmessage-method (when a message is recieved)
    //websocket[id].onerror = function (evt) { onError(evt); };   //function called by websocket.onerror-method (when an error occured)
}

//websocket-action
function onOpen(id) {
    document.getElementById(id).style.backgroundColor = "gray";
    websocket[id].send(0);
    //document.getElementById("disconnect").disabled = false;  //enable "Disconnect"-button
    //document.getElementById("connect").disabled = true; //disable IP-textfield
    //connectedToServer = true;
    //doSend("D:" + Date().substr(16,8));    //Senden der Uhrzeit
}

//websocket-action
function onClose() {
    document.getElementById(id).style.backgroundColor = "black";
    //document.getElementById("connect").disabled = false;
    //document.getElementById("disconnect").disabled = true;
    //connectedToServer = false;
}

//Disconnecting from WebSocket by pressing button
function disconnect() {
    for (var i = 1; i < length.ip; i++){
        websocket[i].close();
        document.getElementById(i).style.backgroundColor = "black";
    }
}

//write message to screen and send it via websocket (instantly)
function doSend(message,id) {
    websocket[id].send(message);
}



//websocket-action
function onMessage(evt) {
    // Einfache Message
    // 0 = RGB(white)
    // 1 = Red
    // 2 = Green
    if (evt.data === "--serverIdle--") {
        serverIdle = true;
        doSend_buffered("");    //try to send buffered message (if it exists)
    } else if ("L" === evt.data.substr(0,1)) {  //if first letter is "B": button-message recieved
        if ("0" === evt.data.substr(1, 1)) { //button zero event (right)
            document.getElementById("Button0Val").innerHTML = evt.data.substr(3, 1);    //write state on website
            if ("1" === evt.data.substr(3, 1))
                document.getElementById("Button0Box").style.backgroundColor = "#00ff00";
            if ("0" === evt.data.substr(3, 1))
                document.getElementById("Button0Box").style = document.getElementsByClassName("mainBoarderBox").style;
        }
        if ("1" === evt.data.substr(1, 1)) { //button one event (left)
            document.getElementById("Button1Val").innerHTML = evt.data.substr(3, 1);    //write state on website
            if ("1" === evt.data.substr(3, 1))
                document.getElementById("Button1Box").style.backgroundColor = "#00ff00";
            if ("0" === evt.data.substr(3, 1))
                document.getElementById("Button1Box").style = document.getElementsByClassName("mainBoarderBox").style;
        }
    } else {    //unknown data are written to screen
        writeToScreen('<span style="color: blue;">RESPONSE: ' + evt.data + '</span>');
    }
    /*
    if (evt.data == '10')
    {
        websocket.close();
    }
    */
}

//websocket-action
function onError(evt) {
    writeToScreen('<span style="color: red;">ERROR:</span> ' + evt.data);
}

//put message-data into buffer. Send this buffer if server is idle.
function doSend_buffered(message){
    //only handle if client is connected to Server
    if (connectedToServer) {
        //put message-data only, if it exists.
        if (message !== "") {
            outputDataStream = outputDataStream + message + ";";
        }
        //send, if server is idle and buffer is not empty
        if ((true === serverIdle) && ("" !== outputDataStream)) {
            websocket.send(outputDataStream);
            outputDataStream = "";  //empty stream
            serverIdle = false; //reset serverIdle-flag
        }
    }
}