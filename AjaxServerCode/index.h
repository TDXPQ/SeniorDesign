const char MAIN_page[] PROGMEM = R"=====(
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
      color: blue;
      margin-top: 10px;
      justify-content: center;
      align-items: center;
      text-align: center;
      font-size: 60px;
    }
    /* Horizontal list of values */
    #list-2 {
      display: flex;
      justify-content: center;
      align-items: center;
      color: blue;
      margin-top: 10px;
      font-size: 60px;
      text-align: center;
    }
    /* Thin gray line */
    #line-1 {
      border-top: 10px solid gray;
      width: 80%;
      margin: 60px auto;
    }
    /* Solid black line */
    #line-2 {
      border-top: 20px solid black;
      width: 80%;
      margin: 10px auto;
    }
    /* Buttons */
    .button {
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
    /* Third list of values */
    #list-3 {
      display: flex;
      flex-direction: column;
      align-items: center;
      color: blue;
      margin-top: 10px;
      font-size: 60px;
      text-align: center;
    }
  </style>
</head>
<body>
  <button id="banner" onclick="getConfigPage()">EVIM Remote</button>
  <div id="list-1">
    <div id="divPackVoltageValue">Pack Voltage: <span id="packVoltageValue">DEFAULT</span></div>
    <div id="divPackCurrentValue">Pack Current: <span id="packCurrentValue">DEFAULT</span></div>
    <div id="divPackSOCValue">Pack SoC: <span id="packSOCValue">DEFAULT</span</div>
    <div id="divPackPowerValue">Pack Power: <span id="packPowerValue">DEFAULT</span></div>
  </div>
  <div id="line-1"></div>
  <div id="list-2">
    <div id="divVoltageValue1" style="margin-right: 40px;"><span id="voltageValue1">190V</span> V</div>
    <div id="divVoltageValue2" style="margin-right: 40px;"><span id="voltageValue2">DEFAULT</span> V</div>
    <div id="divVoltageValue3"><span id="voltageValue3">DEFAULT</span> V</div>
  </div>
  <div id="line-1"></div>
  <div id="list-3">
    <div id="divMotorValue">Motor: <span id="motorValue">DEFAULT</span> &#8457</div>
    <div id="divControllerValue">Controller: <span id="controllerValue">DEFAULT</span> &#8457</div>
    <div id="divDCDCValue">DC-DC: <span id="dcdcValue">DEFAULT</span> &#8457</div>
    </div>
  <div id="line-1"></div>
   <div id="list-3"> 
    <div id="divBBoxValue1">BBox 1: <span id="bboxValue1">DEFAULT</span> </div>
    <div id="divBBoxValue2">BBox 2: <span id="bboxValue2">DEFAULT</span> </div>
    <div id="divAmbientValue">Ambient: <span id="ambientValue">DEFAULT</span> </div>
    </div>

    <script>

      setInterval(function() {
        // Call a function repetatively with 2 Second interval
        getPackVoltage();
        getPackCurrent();
        getPackSOC();
        //getPackPower();
        getVoltage1();
        getVoltage2();
        getVoltage3();
        getMotorValue();
        getControllerValue();
        getDCDCValue();
        getBBoxValue1();
        getBBoxValue2();
        getAmbientValue();
        getThresholdValue();
      }, 500); //1000mSeconds update rate
      
      var thresholdSet = "";
      const THRESHOLD_VALUE = 178;

      function getThresholdValue() {
        
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            thresholdSet = this.responseText;
            // Segement of code for threshold indication (i.e. beeping and color change)
            //console.log(thresholdSet.localeCompare("LOW") == 0);
            //console.log(parseInt((document.getElementById("packVoltageValue").innerHTML.slice(0,-1))) >= THRESHOLD_VALUE);
            if ((thresholdSet.localeCompare("LOW")) == 0){
              if(parseInt((document.getElementById("packVoltageValue").innerHTML.slice(0,-1))) >= THRESHOLD_VALUE){
                document.getElementById("packVoltageValue").style.color = "red";
                document.getElementById("divPackVoltageValue").style.color = "red";
                // PUT AUDIO ALERT HERE
                // PUT AUDIO ALERT HERE
                // PUT AUDIO ALERT HERE
                // PUT AUDIO ALERT HERE   



              } else {
                document.getElementById("packVoltageValue").style.color = "blue";
                document.getElementById("divPackVoltageValue").style.color = "blue";
              }
            } else {
              document.getElementById("packVoltageValue").style.color = "blue";
              document.getElementById("divPackVoltageValue").style.color = "blue";
            }

          }
        };
        xhttp.open("GET", "readThreshold", true);
        xhttp.send();
      }

      function getPackVoltage() {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            document.getElementById("packVoltageValue").innerHTML =
            this.responseText.slice(5);
          }
        };
        xhttp.open("GET", "readPackVoltage", true);
        xhttp.send();
      }
      function getPackCurrent() {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            document.getElementById("packCurrentValue").innerHTML =
            this.responseText.slice(5);
          }
        };
        xhttp.open("GET", "readPackCurrent", true);
        xhttp.send();
      }
      function getPackSOC() {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            document.getElementById("packSOCValue").innerHTML =
            this.responseText.slice(5);
          }
        };
        xhttp.open("GET", "readSOCValue", true);
        xhttp.send();
      }
      function getPackPower() {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            document.getElementById("packPowerValue").innerHTML =
            this.responseText.slice(5);
          }
        };
        xhttp.open("GET", "readPowerValue", true);
        xhttp.send();
      }
      function getVoltage1() {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            document.getElementById("voltageValue1").innerHTML =
            this.responseText;
          }
        };
        xhttp.open("GET", "readvoltageValue1", true);
        xhttp.send();
      }
      function getVoltage2() {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            document.getElementById("voltageValue2").innerHTML =
            this.responseText;
          }
        };
        xhttp.open("GET", "readvoltageValue2", true);
        xhttp.send();
      }
      function getVoltage3() {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            document.getElementById("voltageValue3").innerHTML =
            this.responseText;
          }
        };
        xhttp.open("GET", "readvoltageValue3", true);
        xhttp.send();
      }
      function getMotorValue() {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            document.getElementById("motorValue").innerHTML =
            this.responseText;
          }
        };
        xhttp.open("GET", "readMotorValue", true);
        xhttp.send();
      }
      function getControllerValue() {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            document.getElementById("controllerValue").innerHTML =
            this.responseText;
          }
        };
        xhttp.open("GET", "readControllerValue", true);
        xhttp.send();
      }
      function getDCDCValue() {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            document.getElementById("dcdcValue").innerHTML =
            this.responseText;
          }
        };
        xhttp.open("GET", "readDCDCValue", true);
        xhttp.send();
      }
      function getBBoxValue1() {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            document.getElementById("bboxValue1").innerHTML =
            this.responseText;
          }
        };
        xhttp.open("GET", "readBBoxValue1", true);
        xhttp.send();
      }
      function getBBoxValue2() {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            document.getElementById("bboxValue2").innerHTML =
            this.responseText;
          }
        };
        xhttp.open("GET", "readBBoxValue2", true);
        xhttp.send();
      }
      function getAmbientValue() {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            document.getElementById("ambientValue").innerHTML =
            this.responseText;
          }
        };
        xhttp.open("GET", "readAmbientValue", true);
        xhttp.send();
      }

      function getConfigPage() {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          location.reload();
        }
        };
        xhttp.open("GET", "/readConfigPage", true);
        xhttp.send();
      }

    </script> 
</body>
</html>
)=====";