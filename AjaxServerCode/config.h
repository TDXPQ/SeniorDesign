const char CONFIG_page[] PROGMEM = R"=====(
<html>
<head>
  <style>
    /* Blue banner at the top of the website */
    #banner {
      background-color: #41AEEE;
      color: white;
      text-align: center;
      font-size: 70px;
      padding: 40px;
      border: solid 2px black;
      border-radius: 5px;
      margin-bottom: 90px;
      width: 100%;
    }
    /* Vertical list of values */
    #list-1 {
      display: flex;
      flex-direction: column;
      align-items: center;
      color: black;
      margin-top: 10px;
      text-align: center;
    }
    /* Horizontal list of values */
    #list-2 {
      display: flex;
      justify-content: center;
      align-items: center;
      color: black;
      margin-top: 10px;
      text-align: center;
    }
    /* Thin gray line */
    #line-1 {
      border-top: 10px solid gray;
      width: 80%;
      margin: 90px auto;
    }
    /* Solid black line */
    #line-2 {
      border-top: 10px solid black;
      width: 100%;
      margin: 50px auto;
    }
    /* Buttons */
    #button-2 {
      background-color: #41AEEE;
      border-radius: 20px; 
      color: black;
      font-size: 70px;
      padding: 40px;
      border: solid 5px black;
      margin: 35px;
      width: 850px;
      height: 225px;

    }
    /* Third list of values */
    #list-3 {
      display: flex;
      flex-direction: column;
      align-items: center;
      color: black;
      margin-top: 10px;
      text-align: center;
    }
  </style>
</head>
<body>
  <button id="banner" onclick="getRemotePage()">EVIM Configuration</button>
  <div id="list-1">
    <button id="button-2" onclick="getWifiCredPage()">Modify Wi-Fi Access Points Credentials</button>
    <button id="button-2" onclick="getWifiReconnect()">Network Reconnect</button>
    <button id="button-2" onclick="getRestartServer()">Cycle Relay</button>
  </div>
  <div id="line-2"></div>
  <div id="list-3">
    <div id="divIpAddressValue" style="font-size: 70px;margin: 20px;">IP Address: <span id="ipAddressValue">Test</span></div>
    <div id="divCurrentDNSValue" style="font-size: 70px;margin: 20px;">Curent DNS: <span id="currentDNSValue">Test</span></div>
    <div id="divNetMaskValue" style="font-size: 70px;margin: 20px;">Net Mask: <span id="netMaskValue">Test</span></div>
    </div>
  <div id="line-2"></div>
   <div id="list-3"> 
    <div id="divMACValue" style="font-size: 70px;">MAC = <span id="MACValue">Test</span> </div>
    </div>

    <script>

      setInterval(function() {
        getIpAddress();
        getDNSValue();
        getNetMaskValue();
        getMACValue();
      }, 500); //2000mSeconds update rate
      
      function getIpAddress() {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            document.getElementById("ipAddressValue").innerHTML =
            this.responseText;
          }
        };
        xhttp.open("GET", "readIpAddressValue", true);
        xhttp.send();
      }

      function getDNSValue() {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            document.getElementById("currentDNSValue").innerHTML =
            this.responseText;
          }
        };
        xhttp.open("GET", "readDNSValue", true);
        xhttp.send();
      }

      function getNetMaskValue() {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            document.getElementById("netMaskValue").innerHTML =
            this.responseText;
          }
        };
        xhttp.open("GET", "readNetMaskValue", true);
        xhttp.send();
      }

      function getMACValue() {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            document.getElementById("MACValue").innerHTML =
            this.responseText;
          }
        };
        xhttp.open("GET", "readMACValue", true);
        xhttp.send();
      }

      function getRestartServer() {
        var xhttp = new XMLHttpRequest();
        xhttp.open("GET", "readRestartServer", true);
        xhttp.send();
      }
      
      function getRemotePage() {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          location.reload();
        }
        };
        xhttp.open("GET", "/readRemotePage", true);
        xhttp.send();
      }

      function getWifiCredPage() {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          location.reload();
        }
        };
        xhttp.open("GET", "/readWifiCredPage", true);
        xhttp.send();
      }

      function getWifiReconnect() {
        
        console.log("Reconnect Button Pressed");
        
        var xhttp = new XMLHttpRequest();
        xhttp.open("GET", "readWifiReconnect", true);
        xhttp.send();

      }

      </script> 
</body>
</html>
)=====";