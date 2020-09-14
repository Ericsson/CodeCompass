$(document).ready(function() {
    $.ajax({
        url: 'ga.txt',
        dataType: 'text',
        success: function (gaId) {
            console.log('Google Analytics enabled: ' + gaId);
            $.getScript('https://www.googletagmanager.com/gtag/js?id=' + gaId);

            window.dataLayer = window.dataLayer || [];

            function gtag() {
                dataLayer.push(arguments);
            }

            gtag('js', new Date());
            gtag('config', gaId);
        },
        statusCode: {
            404: function () {
                console.log('Google Analytics disabled.');
            }
        }
    });
});
