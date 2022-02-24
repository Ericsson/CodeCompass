var gtag = null;
$(document).ready(function() {
    $.ajax({
        url: 'ga.txt',
        dataType: 'text',
        success: function (gaId) {
            $.getScript('https://www.googletagmanager.com/gtag/js?id=' + gaId)
            .done(function (script, textStatus){
                console.log('Google Analytics enabled: ' + gaId);
                
                window.dataLayer = window.dataLayer || [];

                gtag = function() {
                    dataLayer.push(arguments);
                }

                gtag('js', new Date());
                gtag('config', gaId);
            })
            .fail(function (jqxhr, settings, exception) {
                console.log('Failed to connect to Google Tag Manager. Google ' +
                    'Analytics will not be enabled.');
            });
        },
        statusCode: {
            404: function () {
                console.log('Google Analytics disabled.');
            }
        }
    });
});
