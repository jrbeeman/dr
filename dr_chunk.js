/**
 * Useful color selector: http://www.colorjack.com/sphere/
 */

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
    if (context == 'page-node') {
        if (Drupal.settings.dr && Drupal.settings.dr.terms) {
            var out = Drupal.dr_chunk.renderStyles();
            if (!Drupal.dr_chunk.loadedStyles) {
                $('head').append(out);
                Drupal.dr_chunk.prepare_node(null);
                Drupal.dr_chunk.loadedStyles = true;
            }
        }
    }
}

/**
 * Note: This won't work with TinyMCE after version 3.2.7
 * @todo Get working with new versions of TinyMCE
 * @todo load all the html in here via AHAH
 * @todo Add a toggle to turn on/off the styles
 */
Drupal.dr_chunk.prepare_node = function(instanceId) {
    if (Drupal.settings.dr && Drupal.settings.dr.terms) {
        var cookie_opts = {path: '/'};
        var terms = Drupal.settings.dr.terms;
        if (!Drupal.dr_chunk.loadedStyles) {
            $('body').append(Drupal.settings.dr.terms_markup);
        }
        $('#dr-chunk-menu-inner a').bind('click', function() {
            if (instanceId) {
                var css_class = $(this).find('span').attr('id');
                var ed = tinyMCE.editors[instanceId];
                ed.execCommand('mceSetCSSClass', 0, css_class);
            }
            return false;
        });
        if (!Drupal.dr_chunk.loadedStyles) {
            $('#dr-chunk-menu-slider a').bind('click', function() {
                $('#dr-chunk-menu-inner').slideToggle(function() {
                  $.cookie('dr-chunk-slider-state', $(this).css('display'), cookie_opts);
                });
                return false;
            });
            $('#dr-chunk-menu .dr-chunk-vocab-name').bind('click', function() {
              $(this).siblings('ul').slideToggle(function() {
                $.cookie($(this).siblings('span.dr-chunk-vocab-name').attr('id'), $(this).css('display'), cookie_opts);
              });
            });
            // Set the initial state of sliders
            $('#dr-chunk-menu-inner').css('display', $.cookie('dr-chunk-slider-state'));
            $('#dr-chunk-menu .dr-chunk-vocab-name').siblings('ul').each(function() {
                $(this).css('display', $.cookie($(this).siblings('span.dr-chunk-vocab-name').attr('id')));
            });       
        }
        // Inject the CSS needed for rendering into the iframe
        // From: http://www.shawnolson.net/a/503/altering-css-class-attributes-with-javascript.html
        // Add local stylesheet to the iframe
        if (instanceId) {
            var out = Drupal.dr_chunk.renderStyles(); 
            $('#'+ instanceId +'_ifr').contents().find('head').append(out);
        }
    }
}

Drupal.dr_chunk.renderStyles = function() {
    var terms = Drupal.settings.dr.terms;
    var out = '<style type="text/css">';
    out += '.content { font: 13px/20px "Helvetica Neue",Helvetica,Arial,sans-serif } ';
    for (index in terms) {
        var term = terms[index];
        var className = 'taxonomy-term-'+ term.tid;
        out += '.'+ className +' { background-color:'+ term.background +'; color:'+ term.foreground +'; } ';
    }
    out += '</style>';
    return out;
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
