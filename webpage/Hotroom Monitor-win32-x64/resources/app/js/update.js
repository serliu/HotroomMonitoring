var chart;
var chart_interval, table_interval, ip_interval;
var upper_thresh = 28.00;
var lower_thresh = 18.00;
function update_chart_table() {
    $.ajax({
        url: "http://30.30.30.90/json",
        data: { tag: 'GetDataFromArduino'},
        dataType: "json",
        timeout: 10000,
        success: function(data){
//            console.log(data.date, data.S0, data.S1);
            var datetime = data.date; //"date time"
            datetime = datetime.split(" "); //[date, time]
            var date = datetime[0].split("-");
            var time = datetime[1].split(":");
            datetime = Date.UTC(date[2], date[0] - 1, date[1], time[0], time[1], time[2]);
            //console.log("UTC TIME: ", datetime);
            var point0 = [datetime, data.S0];
            var point1 = [datetime, data.S1];
//            console.log(point0, point1);
            var series0 = chart.series[0],
                shift0 = series0.data.length > 500;
            var series1 = chart.series[1],
                shift1 = series1.data.length > 500;
            chart.series[0].addPoint(point0, true, shift0);
            chart.series[1].addPoint(point1, true, shift1);
            update_table();
        },
        cache: false
    })
};
function update_table() {
    $.ajax({
        url: "http://30.30.30.90/json",
        data: { tag: 'GetDataFromArduino'},
        dataType: "json",
        timeout: 10000,
        success: function(data){
            document.getElementById('sensor0').innerHTML = data.S0 + '&deg;C';
            document.getElementById('sensor1').innerHTML = data.S1 + '&deg;C';
            if (data.S0 > upper_thresh) document.getElementById('sensor0').setAttribute('class','red');
            else document.getElementById('sensor0').setAttribute('class','green');
            if (data.S1 > upper_thresh) document.getElementById('sensor1').setAttribute('class','red');
            else document.getElementById('sensor1').setAttribute('class','green');
        },
        cache: false
    })
};

var all_ips = [], bad_ips = [];
function get_ip_list(){
    $.get('iplist.txt', function(data){
        all_ips = [];
        all_ips = data.split(/\n/g);
    });
    $.get('badips.txt', function(data){
        bad_ips = [];
        bad_ips = data.split(/\n/g);
    });
    display_iplist();
}
            
function display_iplist(){
    //BAD IP TXT FILE NEEDS NEW LINE AT THE END OR LAST ONE WILL NOT WORK CORRECTLY
    //console.log(all_ips, bad_ips);
    var iptable = "<table><tr>";
    var isbad = false;
    for (var i = 0; i < all_ips.length; i++){
        if ( i % 4 == 0){
            iptable += "</tr><tr>";
        }
        for (var j = 0; j < bad_ips.length; j++){
            //console.log(all_ips[i], "compared to: ", bad_ips[j]);
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
    document.getElementById("s_button").setAttribute('onClick', 'stop_running();')
    update_chart_table()
    chart_interval = setInterval(update_chart_table, 60000);
    table_interval = setInterval(update_table, 10000);
    get_ip_list();
    setInterval(get_ip_list, 10000);
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
            spacingRight: 20
        },

        title: {
            text: 'Temperature Values Recorded by Arduino'
        },

        subtitle: {
            text: 'Click and drag in the plot area to zoom in'
        },

        xAxis: {
            type: 'datetime',
            maxZoom: 2 * 3600000,
            max: null
        },

        yAxis: {
            title: {
                text: 'Temperature (Celcius)'
            },
            min: 0,
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
                point: {
                    events: {
                        click: function() {
                            hs.htmlExpand(null, {
                                pageOrigin: {
                                    x: this.pageX,
                                    y: this.pageY
                                },
                                headingText: this.series.name,
                                maincontentText: Highcharts.dateFormat('%H:%M - %b %e, %Y', this.x) +':<br/> '+
                                    this.y,
                                width: 200
                            });
                        }
                    }
                },
            }
        },

        series: [{
            name: 'Sensor 1',
            marker: {
                radius: 2
            },
            data: []
        },
        {
            name: 'Sensor 2',
            marker: {
                radius: 2
            },
            data: []
        }]
    };

    chart = new Highcharts.Chart(options);
    chart = new Highcharts.Chart(options);
    start_running();

});
    