
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
       document.getElementById("data").innerHTML = '&nbsp;<i class="fa fa-bell-o"></i> '+ datetime + ' # ' +topic+' <i class="fa fa-ellipsis-h"></i> ' + msg;
    }

    microgear.on('message',function(topic,msg) {
        printMsg(topic,msg);
        if (topic == "/MELON/time") {
            $('#time').text(msg);
        }
        
        if (topic == "/MELON/status") {
           var vals = msg.split(",");
//           console.log(vals);
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
 

    });

    microgear.on('connected', function() {
        printMsg('Init',"Connected to NETPIE...");
        microgear.setAlias(ALIAS);
        microgear.subscribe("/cmd");
        microgear.subscribe("/set");
        microgear.subscribe("/time");
        microgear.subscribe("/settime");
        microgear.subscribe("/status");
    });

    microgear.on('present', function(event) {
//        printMsg(event.alias,event.type);
        console.log(event);
    });

    microgear.on('absent', function(event) {
//        printMsg(event.alias,event.type);
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
        console.log("wtr");
        microgear.publish ("/wtr", "1");
    });

    $("#sp_submit").click(function () {
        console.log($('#pa').val()+','+$('#pb').val()+','+$('#mv').val()+','+$('#mv').val());
        microgear.publish("/set/pa",$('#pa').val());
        microgear.publish("/set/pb",$('#pb').val());
        microgear.publish("/set/mv",$('#mv').val());
        microgear.publish("/set/wv",$('#wv').val());
    });

    
$('.timepicker').timepicker({
    timeFormat: 'h:mm p',
    interval: 30,
//    minTime: '10',
//    maxTime: '6:00pm',
//    defaultTime: '11',
    startTime: '08:00',
    dynamic: true,
    dropdown: true,
    scrollbar: true
});