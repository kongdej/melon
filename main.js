
    const APPID = "MELON";
    const KEY = "vjOff5kIQYGig7O";
    const SECRET = "Z8Nm35ZkcqOLupSaDlimEvxUB";

    const ALIAS = "htmlois";

    var microgear = Microgear.create({
        key: KEY,
        secret: SECRET,
        alias : ALIAS
    });

    function printMsg(topic,msg) {
        var now = new Date();
        var d = now.getDay();
        var m = now.getMonth();
        m += 1;  
        var y = now.getFullYear();
        var h = now.getHours();
        var i = now.getMinutes();
        var s = now.getSeconds();
        var datetime = d + "/" + m + "/" + y + " " + h + ":" + i + ":" + s;
//       document.getElementById("data").innerHTML = '&nbsp;<i class="fa fa-bell-o"></i> '+ datetime + ' # ' +topic+' <i class="fa fa-ellipsis-h"></i> ' + msg;
       document.getElementById("data").innerHTML = '&nbsp;<i class="fa fa-bell-o"></i> '+topic+' <i class="fa fa-ellipsis-h"></i> ' + msg;
    }

    function az(str) {
      if (str.length == 1)
        return '0'+str;
      else 
        return str

    }

    microgear.on('message',function(topic,msg) {
        //printMsg(topic,msg);
        if (topic == "/MELON/time") {
            var txts = msg.split(' ');
            var dates = txts[0].split('/');
            var times = txts[1].split(':');
            var str = az(times[0])+':'+az(times[1])+':'+az(times[2])+"<br>"
            str += dates[2]+'/'+dates[1]+'/'+dates[0];
            //console.log(str);
            $('#time').html(str);

            //printMsg(topic,msg);
        }
        
        if (topic == "/MELON/status") {
           var vals = msg.split(",");
//           printMsg(topic,msg);
           if (vals[0] == '1') $('#r1_status').text('START'); else $('#r1_status').text('STOP');                
           if (vals[1] == '1') $('#r2_status').text('START'); else $('#r2_status').text('STOP');                
           if (vals[2] == '1') $('#r3_status').text('OPEN'); else $('#r3_status').text('CLOSE');                
           if (vals[3] == '1') $('#r4_status').text('OPEN'); else $('#r4_status').text('CLOSE'); 
           $('#pa_status').text(vals[4]);
           $('#pb_status').text(vals[5]);
           $('#mv_status').text(vals[6]);
           $('#wv_status').text(vals[7]);
           $("#pa").attr("value", vals[4]);
           $("#pb").attr("value", vals[5]);
           $("#mv").attr("value", vals[6]);
           $("#wv").attr("value", vals[7]);
         }
         if (topic == "/MELON/status/sch/ab") {
           var vals = msg.split(",");
//           printMsg(topic,msg);         
           $("#ab1").attr("value", vals[0]);
           $("#ab2").attr("value", vals[1]);
           $("#ab3").attr("value", vals[2]);
           $("#ab4").attr("value", vals[3]);
           $("#ab5").attr("value", vals[4]);
           $("#ab6").attr("value", vals[5]);
           $("#ab7").attr("value", vals[6]);
           $("#ab8").attr("value", vals[7]);
         }

         if (topic == "/MELON/status/sch/wt") {
           var vals = msg.split(",");
           //printMsg(topic,msg);         
           $("#wt1").attr("value", vals[0]);
           $("#wt2").attr("value", vals[1]);
           $("#wt3").attr("value", vals[2]);
           $("#wt4").attr("value", vals[3]);
           $("#wt5").attr("value", vals[4]);
           $("#wt6").attr("value", vals[5]);
           $("#wt7").attr("value", vals[6]);
           $("#wt8").attr("value", vals[7]);
         }
    });

    microgear.on('connected', function() {
        printMsg('Init',"Connected to NETPIE...");
        microgear.setAlias(ALIAS);
        microgear.subscribe("/cmd");
        microgear.subscribe("/set");
        microgear.subscribe("/time");
        microgear.subscribe("/settime");
        microgear.subscribe("/status/#");
    });

    microgear.on('present', function(event) {
        printMsg(event.alias,event.type);
        console.log(event);
    });

    microgear.on('absent', function(event) {
        printMsg(event.alias,event.type);
        console.log(event);
    });

    microgear.resettoken(function(err) {
        microgear.connect(APPID);
    });

    $("#r1on").click(function () {
        console.log("on");
        microgear.publish ("/cmd", "01");
    });

    $("#r1off").click(function () {
        console.log("off");
        microgear.publish ("/cmd", "00");
    });

    $("#r2on").click(function () {
        console.log("on");
        microgear.publish ("/cmd", "11");
    });

    $("#r2off").click(function () {
        console.log("off");
        microgear.publish ("/cmd", "10");
    });


    $("#r3on").click(function () {
        console.log("on");
        microgear.publish ("/cmd", "21");
    });

    $("#r3off").click(function () {
        console.log("off");
        microgear.publish ("/cmd", "20");
    });

    $("#r4on").click(function () {
        console.log("on");
        microgear.publish ("/cmd", "31");
    });

    $("#r4off").click(function () {
        console.log("off");
        microgear.publish ("/cmd", "30");
    });

    $("#wtr").click(function () {
        microgear.publish ("/wtr", "1");
    });

    $("#rst").click(function () {
        microgear.publish ("/reset", "1");
    });

    $("#sp_submit").click(function () {
      var settings = $('#pa').val()+','+$('#pb').val()+','+$('#mv').val()+','+$('#wv').val();
      console.log(settings);
      microgear.publish("/set/sp",settings);
    });

    $("#sch_submit").click(function () {
      var sch_ab = $('#ab1').val()+','+$('#ab2').val()+','+$('#ab3').val()+','+$('#ab4').val()+','+$('#ab5').val()+','+$('#ab6').val()+','+$('#ab7').val()+','+$('#ab8').val();
      var sch_wt = $('#wt1').val()+','+$('#wt2').val()+','+$('#wt3').val()+','+$('#wt4').val()+','+$('#wt5').val()+','+$('#wt6').val()+','+$('#wt7').val()+','+$('#wt8').val();
      settings = sch_ab+',';
      settings += sch_wt;
      console.log(settings);
      microgear.publish("/set/sch",settings);
    });

$('.clockpicker').clockpicker();