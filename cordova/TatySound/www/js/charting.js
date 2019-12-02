
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
      text: 'Sound Dose'
  },
  xAxis: {
      type: 'datetime',
      tickPixelInterval: 150
  },
  yAxis: {
      title: {
          text: 'Acc (mg)'
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
      name: 'X',
      data: []
  }, {
      name: 'Y',
      data: []
  }, {
      name: 'Z',
      data: []
  }],
});