const char CREDENTIAL_page[] PROGMEM = R"=====(
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
    }
    /* Horizontal list of values */
    #list-2 {
      display: flex;
      justify-content: center;
      align-items: center;
      color: black;
      margin-top: 10px;
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
      width: 80%;
      margin: 90px auto;
    }
    /* Buttons */
    #button {
      background-color: #41AEEE;
      border-radius: 20px; 
      color: black;
      font-size: 70px;
      padding: 40px;
      border: solid 5px black;
      margin: 40px;
      text-align: center;
      display: block;
      margin: 0 auto;
    }
    /* Third list of values */
    #list-3 {
      display: flex;
      flex-direction: column;
      align-items: center;
      color: black;
      margin-top: 10px;
    }
    table {
        margin: 0 auto; /* centers the table horizontally */
    }    

    input[type="text"] {
        
        width: 325px; /* sets the width of the input fields */
        height: 100px; /* sets the height of the input fields */
        font-size: 35px; /* sets the font size of the text in the input fields */
        padding: 10px; /* adds some padding around the input fields */
        box-sizing: border-box; /* includes padding in the element's total width/height */
        box-sizing: border-box; /* includes padding in the element's total width/height */
        border: solid 5px black; /* adds a border around the input fields */
        border-radius: 5px; /* adds rounded corners to the input fields */
        margin: 40px;
        margin-left: 25px;
    }
  </style>
</head>
<body onload="getNetworkCreds()">
    <button id="banner" onclick="getConfigPage()">Wi-Fi Access Points Credentials</button>
    
    <table>
        <tr>
            <th style="font-size: 40px;">Priority</th>
            <th style="font-size: 40px;">Network Name</th>
            <th style="font-size: 40px;">Network Password</th>
        </tr>
        <tr>
          <td style="font-size: 80px;">1.</td>
          <td><input type="text" id="networkName1"></td>
          <td><input type="text" id="networkPassword1"></td>
        </tr>
        <tr>
          <td style="font-size: 80px;">2.</td>
          <td><input type="text" id="networkName2"></td>
          <td><input type="text" id="networkPassword2"></td>
        </tr>
        <tr>
          <td style="font-size: 80px;">3.</td>
          <td><input type="text" id="networkName3"></td>
          <td><input type="text" id="networkPassword3"></td>
        </tr>
    </table>
      

    <button id="button" onclick="buttonNetworkCredentials()">Save Credentials</button>


    <script>

      setInterval(function() {

      }, 2000); //2000mSeconds update rate

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

      function getNetworkCreds(){
        var xhttp = new XMLHttpRequest();
        
        
        xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var creds = this.responseText;
          
          console.log("Data Retrieval: ");
          console.log(creds);
          
          var credsArray = creds.split(",");

          while(credsArray.length < 6){
            credsArray = credsArray.concat("");
          }
          
          document.getElementById("networkName1").value = credsArray[0];
          document.getElementById("networkPassword1").value = credsArray[1];
          document.getElementById("networkName2").value = credsArray[2];
          document.getElementById("networkPassword2").value = credsArray[3];
          document.getElementById("networkName3").value = credsArray[4];
          document.getElementById("networkPassword3").value = credsArray[5];

        }
        };
        xhttp.open("GET", "readNetworkCreds", true);
        xhttp.send();
      }

      function saveWifiCreds() {
        var xhttp = new XMLHttpRequest();

        xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          //console.log("Done Pressing Button");
        }
        };

        var data = document.getElementById("networkName1").value + "," +
                  document.getElementById("networkPassword1").value + "," +
                  document.getElementById("networkName2").value + "," +
                  document.getElementById("networkPassword2").value + "," +
                  document.getElementById("networkName3").value + "," +
                  document.getElementById("networkPassword3").value;

        xhttp.open("GET", "saveWifiCreds?data=" + data, true);
        xhttp.send();
        //console.log(data);
      }

      function buttonNetworkCredentials(){
        saveWifiCreds();
        getNetworkCreds();
      }

      </script> 
</body>
</html>
)=====";