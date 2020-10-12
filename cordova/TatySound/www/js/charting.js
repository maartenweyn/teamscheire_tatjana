
Highcharts.setOptions({
  global: {
      useUTC: false
  }
});

var sound_chart = Highcharts.chart('plot_container', {
  chart: {
      type: 'line',
      animation: false, 
      marginRight: 10
  },
  navigator: {
    enabled: false
  },
  title: {
      text: ''
  },
  xAxis: {
      type: 'datetime',
      tickPixelInterval: 50
  },
  yAxis: {
      title: {
          text: 'Sound (dbA)  / Sound Dose (%)'
      },
      plotLines: [{
          value: 0,
          width: 1,
          color: '#808080'
      }]
  },
  // tooltip: {
  //     formatter: function () {
  //         return '<b>' + this.series.name + '</b><br/>' +
  //             Highcharts.dateFormat('%Y-%m-%d %H:%M:%S', this.x) + '<br/>' +
  //             Highcharts.numberFormat(this.y, 2);
  //     }
  // },
  legend: {
      enabled: true
  },
  exporting: {
      enabled: false
  }, 
  plotOptions: {
    series: {
        marker: {
            enabled: false   
        }
    }
  },

  series: [{
      name: 'Avg Minute',
      color: '#666666',
      data: []
  }, {
      name: 'Avg Hour',
      color: '#229988',
      data: []
  }, {
      name: 'Avg 8 Hours',
      color: '#9c27b0',
      data: []
  }, {
    name: 'Avg Day',
    color: '#F9BE1F',
    data: []
}, {
  name: '8 Hour Dose (%)',
  color: '#ffc107',
  data: []
}, {
  name: 'Day dose (%)',
  color: '#ff5722',
  data: []
}],
});

var sound_bar = Highcharts.chart('bar_container', {
    chart: {
        height: 100,
        type: 'bar',
        credits: false
    },
    exporting: {
        enabled: false
    }, 
    legend: {
        enabled: false
    },
    title: {
        text: ''
    },
    xAxis: {
        categories: ['8 Hour', 'Day']
    },
    yAxis: {
        min: 0,
        minRange: 90,
        title: {
            text: 'Noise Dose (%)'
        }
    },
    legend: {
        reversed: true
    },
    plotOptions: {
        series: {
            stacking: 'normal'
        }
    },
    series: [{
        name: 'noise',
        showInLegend: false,
        dataLabels: [{
            inside: false,
                enabled: true,
            style: {
            fontSize: 10,
            color: "#4C3D3D",
            },
            format: '{y} %'
        }],
        data: [
            { y: 0.5, color: '#ffc107'},  
            { y: 0.5, color: '#ff5722'}, 
          ]         
    }]
});