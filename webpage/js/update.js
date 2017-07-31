var chart;
var chart_interval, table_interval, ip_interval;
var upper_thresh = 38.00;
var lower_thresh = 10.00;
var chart_refresh_time = 180000; //3 minutes
var live_table_refresh_time = 5000; //5 seconds
var ip_refresh_time = 100000; //5 minutes
var colors =['#1F271B','#19647E','#28AFB0','#F4D35E','#EE964B','#1EB24F','#0B4F6C','#01BAEF','#757575']
var day = 0;
var days_to_keep_live_data = 5;
var num_ip_colums = 12;




var points = [];
var c = 0;
var datetime, date, time;
function update_chart_table() {
    //Make a request to the arduino for json data containing temperature readings
    $.ajax({
        url: "http://192.168.100.100/json",
        data: { tag: 'GetDataFromArduino'},
        dataType: "json",
        timeout: 3000,
        success: function(data){ //if a request is successful, process the sensor data to add to live plot
            //data is in formate: {date, S0, S1, ...}
            points = [];
            c = 0; //counter variable that will later becme number of sensors
            datetime = data.date;
            datetime = datetime.split(" "); //Splits into this: [date, time]
            date = datetime[0].split("-"); //Splits date
            time = datetime[1].split(":"); //splits time
            datetime = Date.UTC(date[2], date[0] - 1, date[1], time[0], time[1], time[2]); //convert date to UTC format for plotting
            if(day == 0){ //if first run, set the date to current date
                day = date[1];
            }
            if(date[1] != day){ //if today is new day, refresh the log to see new inputs
                refresh_iframe();
                day = date[1];
            }
//            console.log("UTC TIME: ", datetime);
            delete data.date; //remove first point
//            console.log(data);
            for(var x in data){ //make an array of points the number of Sensor data in the json object
                points[c] = [datetime, data[x]];
                c++;
//                console.log(points[c]);
            } //c = NUM_SENSORS NOW
            for(var i = 0; i < c; i++){
                if (chart.series.length == i){ //If there is not a currently existing series for that series, create a new one
                    chart.addSeries({
                        name: ('Sensor ' + i),
                        data: [],
                        color: colors[Math.floor(Math.random()*5)],
                        marker: {
                            enabled: true,
                            radius:1}
                    });
                }
                chart.series[i].addPoint(points[i], true, false); //Add the data point to the corresponding series
                if (chart.series[i].data.length > (days_to_keep_live_data*3600*24*1000/chart_refresh_time)){ //If past days to keep live data, remove first point
                    chart.series[i].removePoint(0, true);
                }
            }
            update_table(); //refesh table

        },
        cache: false
    })
};

function refresh_iframe(){ //resets the frames source to refresh the iframe
    var ifr=document.getElementsByName('arduino_history')[0];
    ifr.src=ifr.src;
}

//Sends request to arduino to update live data chart
var live_table
function update_table() {
    $.ajax({
        url: "http://192.168.100.100/json",
        data: { tag: 'GetDataFromArduino'},
        dataType: "json",
        timeout: 3000,
        success: function(data){
            delete data.date;
            live_table = '<table><tr>';
            //set the headers
            for(var i in data){
                live_table += ('<th>Sensor ' + i[1] + '</th>');
            }
            live_table += '</tr><tr>';
            //check if data is within the thresholds and color them accordingly
            for(var i in data){
                if(data[i] > upper_thresh || data[i] < lower_thresh){
                    live_table += '<td class=red>' + data[i] + '&deg;C</td>';
                }
                else{
                    live_table += '<td class=green>' + data[i] + '&deg;C</td>';
                }
            }
            live_table += '</tr></table>';
            document.getElementById('live_temp_table').innerHTML = live_table; //Create new table with current data
            
            
            
            
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
//from local directory, read the text files and split them by line
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



var iptable = '';
var isbad = false;
function display_iplist(){
    //BAD IP TXT FILE NEEDS NEW LINE AT THE END OR LAST ONE WILL NOT WORK CORRECTLY
    iptable = "<table><tr><th colspan=\"" + num_ip_colums + "\">IP Addresses</th></tr><tr>";
    isbad = false;
    
    for (var i = 0; i < all_ips.length - 1; i++){ //table formatting
        if ( i % num_ip_colums == 0 && i != 0){
            iptable += "</tr><tr>";
        }
        for (var j = 0; j < bad_ips.length; j++){ //iterate through both lists to compare and check for matches
//            console.log(all_ips[i], "compared to: ", bad_ips[j]);
            if (all_ips[i].localeCompare(bad_ips[j]) == 0){
                isbad = true;
                break;
            }
        }
        //Color each ip accordingly, if the ip doesnt have a lenght, just add blank column
        if (all_ips[i].length == 0){
            iptable += "<td>" + all_ips[i]+ "</td>";
        }
        else if(isbad){
            iptable += "<td class=\"red\">" + all_ips[i]+ "</td>";
        } 
        else{
            iptable += "<td class=\"green\">" + all_ips[i]+ "</td>";
        }
        isbad = false;
    }

    iptable += "</tr></table>"
    //console.log(iptable);
    document.getElementById('iptable').innerHTML = iptable; //set the table
}
            
//Set the intervales for updating different elements and button coloring/funciton
function start_running(){
    document.getElementById("s_button").innerHTML = "Stop Updating";
    document.getElementById("s_button").style.backgroundColor = "#FF4136";
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


//stops elements from updating
function stop_running(){
    clearInterval(chart_interval);
    clearInterval(table_interval);
    clearInterval(ip_interval);
    document.getElementById("s_button").innerHTML = "Resume Updating";
    document.getElementById("s_button").style.backgroundColor = "#1EB24F";
    document.getElementById("s_button").setAttribute('onClick', 'start_running();')
    
}
//when ready, create the highcharts options
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
            },
//            borderWidth: 3,
//            borderColor: '#0B4F6C',
            borderRadius: 7,
            backgroundColor: '#FBFBFF'
            
            
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
            min: 18,
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
                enabled: true,
                radius: 1
            },
            data: []
        },
]
    };

    chart = new Highcharts.Chart(options);
    chart = new Highcharts.Chart(options);
    start_running();

});
    