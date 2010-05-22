Drupal.dr_chunk = {};
Drupal.dr_chunk.loadedStyles = false;


Drupal.dr_chunk.initialize = function() {
    // Determine the context
    var context = '';
    if ($('#taxonomy-form-term').length > 0) {
        context = 'taxonomy-form';
    }
    if ($('body.page-node').length > 0) {
        context = 'page-node';
    }

    // Taxonomy form alterations
    if (context == 'taxonomy-form') {
        var fore = $('#taxonomy-form-term #edit-dr-foreground').val();
        var back = $('#taxonomy-form-term #edit-dr-background').val();
        var fb_fore = $.farbtastic('#taxonomy-form-term #colorpicker-foreground', Drupal.dr_chunk.farb_fore);
        fb_fore.setColor(fore);
        var fb_back = $.farbtastic('#taxonomy-form-term #colorpicker-background', Drupal.dr_chunk.farb_back);
        fb_back.setColor(back);
    }

    // Node page alterations
    if (context == 'page-node') {
        //Drupal.dr_chunk.prepare_node();
    }
}

/**
* @todo load all the html in here via AHAH
* @todo Add a toggle to turn on/off the styles
*/
Drupal.dr_chunk.prepare_node = function(instanceId) {
    var terms = Drupal.settings.dr.terms;
    var menu = '<div id="dr-chunk-menu"><div id="dr-chunk-menu-inner">';
    for (index in terms) {
        var term = terms[index];
        menu += '<a href="#">';
        menu += '<span id="taxonomy-term-'+ term.tid +'" style="background-color:'+ term.background +'; color: '+ term.foreground +';">'+ term.name +'</span>';
        menu += '</a>';
    }
    menu += '</div>';
    menu += '<div id="dr-chunk-menu-slider" style="padding:2px; text-align:right;"><a href="#">Show / Hide Tags</a></div>';
    menu += '</div>';
    $('body').append(menu);
    $('#dr-chunk-menu-inner a').bind('click', function() {
        var css_class = $(this).find('span').attr('id');
        var ed = tinyMCE.editors[instanceId];
        ed.execCommand('mceSetCSSClass', 0, css_class);
        return false;
    });
    $('#dr-chunk-menu-slider a').bind('click', function() {
        $('#dr-chunk-menu-inner').slideToggle();
        return false;
    });

    // Inject the CSS needed for rendering into the iframe
    // From: http://www.shawnolson.net/a/503/altering-css-class-attributes-with-javascript.html
    // Add local stylesheet to the iframe
    var out = '<style type="text/css">';
    for (index in terms) {
        var term = terms[index];
        var className = 'taxonomy-term-'+ term.tid;
        out += '.'+ className +' { background-color:'+ term.background +'; color:'+ term.foreground +'; } ';
    }
    out += '</style>';
    $('#'+ instanceId +'_ifr').contents().find('head').append(out);
    if (!Drupal.dr_chunk.loadedStyles) {
        $('head').append(out);
    }
}

/**
 * Foreground callback
 */
Drupal.dr_chunk.farb_fore = function(color) {
    $('#taxonomy-form-term #edit-dr-foreground').val(color);
}

/**
 * Background callback
 */
Drupal.dr_chunk.farb_back = function(color) {
    $('#taxonomy-form-term #edit-dr-background').val(color);
}

if (Drupal.jsEnabled) {
  $(document).ready(function() {
    Drupal.dr_chunk.initialize();
  });
}
