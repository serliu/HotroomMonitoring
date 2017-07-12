var chart;
var chart_interval, table_interval, ip_interval;
var upper_thresh = 50.00;
var lower_thresh = 10.00;
var chart_refresh_time = 180000; //3 minutes
var live_table_refresh_time = 3000; //3 seconds
var ip_refresh_time = 900000; //15 minutes
var colors = ['#1F271B','#19647E','#28AFB0','#F4D35E','#EE964B']
function update_chart_table() {
    $.ajax({
        url: "http://30.30.30.90/json",
        data: { tag: 'GetDataFromArduino'},
        dataType: "json",
        timeout: 3000,
        success: function(data){
            var points = [];
            var c = 0;
            var datetime = data.date;
            datetime = datetime.split(" "); //[date, time]
            var date = datetime[0].split("-");
            var time = datetime[1].split(":");
            datetime = Date.UTC(date[2], date[0] - 1, date[1], time[0], time[1], time[2]);
//            console.log("UTC TIME: ", datetime);
            delete data.date;
//            console.log(data);
            for(var x in data){
                points[c] = [datetime, data[x]];
                c++;
//                console.log(points[c]);
            } //c = NUM_SENSORS NOW
            for(var i = 0; i < c; i++){
                if (chart.series.length == i){
                    chart.addSeries({
                        name: ('Sensor ' + i),
                        data: [],
                        color: colors[Math.floor(Math.random()*5)],
                        marker: {radius:3}
                    });
                }
                chart.series[i].addPoint(points[i], true, false);
                if (chart.series[i].data.length > 3360){
                    chart.series[i].removePoint(0, true);
                }
            }
            update_table();
            
            
////            console.log(data.date, data.S0, data.S1);
//            var datetime = data.date; //"date time"
//            datetime = datetime.split(" "); //[date, time]
//            var date = datetime[0].split("-");
//            var time = datetime[1].split(":");
//            datetime = Date.UTC(date[2], date[0] - 1, date[1], time[0], time[1], time[2]);
//            //console.log("UTC TIME: ", datetime);
//            
//            
//            
//            var point0 = [datetime, data.S0];
//            var point1 = [datetime, data.S1];
////            console.log(point0, point1);
//            var series0 = chart.series[0],
//                shift0 = series0.data.length > 500;
//            var series1 = chart.series[1],
//                shift1 = series1.data.length > 500;
//            chart.series[0].addPoint(point0, true, shift0);
//            chart.series[1].addPoint(point1, true, shift1);
//            if (chart.series[0].data.length > 3360){ //one week divided by 3 minutes
//                chart.series[0].removePoint(0, true);
//                chart.series[1].removePoint(0, true);
//            }
//            update_table();
        },
        cache: false
    })
};
function update_table() {
    $.ajax({
        url: "http://30.30.30.90/json",
        data: { tag: 'GetDataFromArduino'},
        dataType: "json",
        timeout: 3000,
        success: function(data){
            delete data.date;
            var live_table = '<table><tr>';
            for(var i in data){
                live_table += ('<th>Sensor ' + i[1] + '</th>');
            }
            live_table += '</tr><tr>';
            for(var i in data){
                if(data[i] > upper_thresh || data[i] < lower_thresh){
                    live_table += '<td class=red>' + data[i] + '&deg;C</td>';
                }
                else{
                    live_table += '<td class=green>' + data[i] + '&deg;C</td>';
                }
            }
            live_table += '</tr></table>';
            document.getElementById('live_temp_table').innerHTML = live_table;
            
            
            
            
//            document.getElementById('sensor0').innerHTML = data.S0 + '&deg;C';
//            document.getElementById('sensor1').innerHTML = data.S1 + '&deg;C';
//            if (data.S0 > upper_thresh) document.getElementById('sensor0').setAttribute('class','red');
//            else document.getElementById('sensor0').setAttribute('class','green');
//            if (data.S1 > upper_thresh) document.getElementById('sensor1').setAttribute('class','red');
//            else document.getElementById('sensor1').setAttribute('class','green');
        },
        cache: false
    })
};

var all_ips = [], bad_ips = [];

function get_ip_list(){
    $.get('iplist.txt', function(data){
        all_ips = [];
        all_ips = data.split("\n");
    });
    $.get('badips.txt', function(data){
        bad_ips = [];
        bad_ips = data.split("\n");
    });
    display_iplist();
}
            
function display_iplist(){
    //BAD IP TXT FILE NEEDS NEW LINE AT THE END OR LAST ONE WILL NOT WORK CORRECTLY
    //console.log(all_ips, bad_ips);
//    console.log(all_ips)
    var iptable = "<table><tr>";
    var isbad = false;
    for (var i = 0; i < all_ips.length; i++){

        if ( i % 4 == 0){
            iptable += "</tr><tr>";
        }
        for (var j = 0; j < bad_ips.length; j++){
//            console.log(all_ips[i], "compared to: ", bad_ips[j]);
            if (all_ips[i].localeCompare(bad_ips[j]) == 0){
                isbad = true;
                break;
            }
        }
        if(isbad){
            iptable += "<td class=\"red\">" + all_ips[i]+ "</td>";
        } else{
            iptable += "<td class=\"green\">" + all_ips[i]+ "</td>";
        }
        isbad = false;
    }
    iptable += "</tr><table>"
    //console.log(iptable);
    document.getElementById('iptable').innerHTML = iptable;
}
            
            
function start_running(){
    document.getElementById("s_button").innerHTML = "Stop Updating";
    document.getElementById("s_button").style.backgroundColor = "#e33100";
    document.getElementById("s_button").setAttribute('onClick', 'stop_running();');
    update_chart_table();
    display_iplist();
    chart_interval = setInterval(update_chart_table, chart_refresh_time);
    table_interval = setInterval(update_table, live_table_refresh_time);
    setInterval(get_ip_list, ip_refresh_time);
    get_ip_list();

}

window.onload = function (){
    get_ip_list();
}

function stop_running(){
    clearInterval(chart_interval);
    clearInterval(table_interval);
    clearInterval(ip_interval);
    document.getElementById("s_button").innerHTML = "Resume Updating";
    document.getElementById("s_button").style.backgroundColor = "blue";
    document.getElementById("s_button").setAttribute('onClick', 'start_running();')
    
}

$(document).ready(function() {
    // define the options
    var options = {
        global: {
            useUTC: true
        },
        chart: {
            renderTo: 'container',
            zoomType: 'xy',
            spacingRight: 20,
            resetZoomButton: {
                position: {
                    align: 'left',
                    verticalAlign: 'bottom',
                }
            }
        },

        title: {
            text: 'Temperature Values Recorded by Arduino'
        },

        subtitle: {
            text: 'Click and drag in the plot area to zoom in'
        },

        xAxis: {
            type: 'datetime',
            maxRange: 7* 24 * 3600000,
            minRange: 3600000/4
        },

        yAxis: {
            title: {
                text: 'Temperature (Celcius)'
            },
            min: null,
            startOnTick: false,
            showFirstLabel: false,
            max: null
        },

        legend: {
            enabled: true
        },

        tooltip: {
            formatter: function() {
                    return '<b>'+ this.series.name +'</b><br/>'+
                    Highcharts.dateFormat('%H:%M - %b %e, %Y', this.x) +': '+ this.y;
            }
        },

        plotOptions: {
            series: {
                cursor: 'pointer',
                lineWidth: 1.0,
            }
        },

        series: [{
            name: 'Sensor 0',
            marker: {
                radius: 3
            },
            data: []
        },
]
    };

    chart = new Highcharts.Chart(options);
    chart = new Highcharts.Chart(options);
    start_running();

});
    