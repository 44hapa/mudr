<?php
    
?>


<html>
<head>
</head>
<body>
<script type="text/javascript">
   if ("WebSocket" in window) {
       var ws = new WebSocket("ws://127.0.0.1:81/exampleApp");
//       var ws = new WebSocket("ws://echo.websocket.org");
       ws.onopen = function() {
           ws.send("ping");
       // Web Socket подключён. Вы можете отправить данные с помощью метода send().
//        alert('open');
       }
       //каждый раз, когда браузер получает какие-то данные через веб-сокет
      ws.onmessage = function (evt) {
//          alert('Получили сообщение: ' + evt.data );
        document.getElementById('testDiv').textContent += evt.data;
          };
          ws.onclose = function() {
              alert('fuck');
          // websocket закрыт.
          }
  }
  else {
      // для браузеров не поддерживающих WebSocket.
      alert('Socket MUST DIE!!');
  }
  
</script>
    <div style="border-style: dashed; padding: 10px; margin-bottom: 10px;" id="testDiv">TEST: </div>
    <form>
        <input type="text" onkeypress="javascript: ws.send(this.value);" size="150"/>
        <input type="submit" value="PING" onclick='function(){ws.send("ping");}' />
    </form>
    

</body>
</html>