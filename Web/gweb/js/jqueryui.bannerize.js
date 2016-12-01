/* Author: AB, Date: 07/13/2011 */
/* Modified: EU, Date: 12/01/2011, Purpose: changed self.options.interval casting to interpret msecs and secs */
;(function($) {
 $.widget("ma.bannerize", {
  options: {
   interval: 5,
   auto: true,
   shuffle: false,
   startAt: 0,
   overlay: true,
   className: ''
  },
  _create: function(){
   var self = this; // global reference to widget object
   self.rotating = self.options.auto;
   // banner element
   self.element.addClass('ui-banner ' + self.options.className);
   // banner slides
   self.slides = self.element.find('ul:eq(0)')
    .addClass('ui-banner-slides');
   // banner slogans
   self.slogans = self.element.find('ul:eq(1)')
   .addClass('ui-banner-slogans')
   .delegate('li', 'click.bannerize', function(event){
    event.preventDefault();
    var $obj = $(this);
    self.queue("stop");
    if(!$obj.hasClass('ui-banner-slogans-current') && !$obj.hasClass('ui-banner-slogans-prev')){
     self._rotate($obj.prevUntil('.ui-banner-slogans-current').length + 1, "next");
    }
   });
   if(self.options.shuffle){
    self._shuffle();
    self.options.startAt = 0;
   }
   // left arrow
   self.arrowPrev =
    $('<a/>', {
     href: '#'
    })
    .addClass('ui-banner-arrow ui-banner-arrow-prev png_bg')
    .bind('click', function(event){
     event.preventDefault();
     self.queue("stop");
     self._rotate(1, "prev");
    })
    .appendTo(self.element);
   // right arrow
   self.arrowNext =
    $('<a/>', {
     href: '#'
    })
    .addClass('ui-banner-arrow ui-banner-arrow-next png_bg')
    .bind('click', function(event){
     event.preventDefault();
     self.queue("stop");
     self._rotate(1, "next");
    })
    .appendTo(self.element);
   self.arrows = $([]).pushStack([self.arrowPrev[0], self.arrowNext[0]]);
   self.overlay = $('<div class="ui-banner-overlay png_bg"></div>');
   if(self.options.overlay){
    self.overlay.appendTo(self.element);
   }
   if(self.slides.children().length !== self.slogans.children().length){ // ensure banners and slogans have the same amount
    self.element.hide();
    self.destroy();
   }
   if(!self.slides.children(':eq(' + self.options.startAt + ')').length){ // ensure the start at specifed exists
    self._setOption("startAt", 0);
   }
   self.slides
    .children(':eq(' + self.options.startAt + ')')
     .addClass('ui-banner-current ui-banner-slides-current')
     .next()
      .addClass('ui-banner-next ui-banner-slides-next')
     .end()
     .filter(function(i){
       if(!self.options.startAt){
        self.slides
         .children(':last')
          .detach()
          .prependTo(self.slides);
       }
       return true;
     })
     .prev()
      .addClass('ui-banner-prev ui-banner-slides-prev');
   self.slogans
    .children(':eq(' + self.options.startAt + ')')
     .addClass('ui-banner-current ui-banner-slogans-current')
     .next()
      .addClass('ui-banner-next ui-banner-slogans-next')
     .end()
     .filter(function(i){
       if(!self.options.startAt){
        self.slogans
         .children(':last')
          .detach()
          .prependTo(self.slogans);
       }
       return true;
     })
     .prev()
      .addClass('ui-banner-prev ui-banner-slogans-prev')
      .each(function(i, v){
       var $obj = $(this);
       $obj.css('marginTop', ($obj.outerHeight() * -1) + 'px');
      });
   $('body').bind('keypress.bannerize', function(event){ // listen for keypress' that are left or right arrow
    if(/^(input|textarea)$/i.test(event.target.nodeName)){
     return;
    }
    if([37,39].indexOf(event.keyCode) !== -1){
     self.queue("stop");
     self._rotate(1, (event.keyCode === 37) ? "prev" : "next");
    }
   });
   self.options.interval = self.options.interval.toString().split(".")[0].length < 4 ? self.options.interval * 1000 : self.options.interval;
   if(self.options.auto){// start the queue
    self.queue("start");
   }
  },
  _shuffle: function(){
   var self = this, pos, len, $slides = self.slides.children(), slides = [], $slogans = self.slogans.children(), slogans = [];
   while (len = $slides.length, len > 0){
    pos = parseInt(Math.random() * len);
    slides.push($slides[pos]);
    slogans.push($slogans[pos]);
    $slides = $slides.filter(':not(:eq(' + pos + '))');
    $slogans = $slogans.filter(':not(:eq(' + pos + '))');
   }
   self.slides
    .empty()
    .append($([]).pushStack(slides));
   self.slogans
    .empty()
    .append($([]).pushStack(slogans));
   return;
  },
  queue: function(action){
   var self = this;
   if(action === "start"){
    self.element
     .bind("rotate.bannerize", function(event){
      var $obj = $(this);
      $obj
       .delay(self.options.interval, "rotate")
       .queue("rotate", function(next){
        $obj.trigger("rotate");
        next();
       });
       self._rotate(1, "next");
     })
     .delay(self.options.interval, "rotate")
     .queue("rotate", function(next){
       var $obj = $(this);
       $obj.trigger("rotate");
       next();
     })
     .dequeue("rotate");
    self.rotating = true;
   }
   else{
    self.element
     .unbind("rotate.bannerize")
     .clearQueue("rotate");
    self.rotating = false;
   }
  },
  _rotate: function(step, direction){
   var self = this;
   if(self.sliding){
    return;
   }
   (direction === "prev") ? self._slideBack(step) : self._slideForward(step);
  },
  _slideBack: function(step){
   var self = this;
   self.slides.queue("banner", function(next){
    self.sliding = true;
    next();
   });
   for(var a = 1; a <= step; a++){
    self.slides
     .queue("banner", function(next){
      self.slides
       .children('.ui-banner-next')
        .removeClass('ui-banner-next ui-banner-slides-next')
        .css('left', '');
      self.slogans
       .children('.ui-banner-next')
        .removeClass('ui-banner-next ui-banner-slogans-next')
        .css('left', '');
      next();
     })
     .queue("banner", function(next){
      self.slogans
       .children('.ui-banner-current')
        .toggleClass('ui-banner-current ui-banner-slogans-current ui-banner-next ui-banner-slogans-next')
       .end()
       .children('.ui-banner-prev')
        .animate({ marginTop: '0px' }, 250, function(){
         $(this).toggleClass('ui-banner-prev ui-banner-slogans-prev ui-banner-current ui-banner-slogans-current');
         next();
        });
      self.slides
       .children('.ui-banner-current')
        .each(function(i, v){
         var $obj = $(this);
         $obj.animate({
          left: '+=' + $obj.width() + 'px'
         }, 250, function(){
          $obj
           .toggleClass('ui-banner-current ui-banner-slides-current ui-banner-next ui-banner-slides-next')
           .css('left', '');
         })
        })
       .end()
       .children('.ui-banner-prev')
        .animate({ left: '0px' }, 250, function(){
         $(this)
         .toggleClass('ui-banner-prev ui-banner-slides-prev ui-banner-current ui-banner-slides-current')
         .css('left', '');
         next();
        });
     })
     .queue("banner", function(next){
      self.slogans
       .children(':last')
        .addClass('ui-banner-prev ui-banner-slogans-prev')
        .each(function(i, v){
         var $obj = $(this);
         $obj.css('marginTop', ($obj.outerHeight() * -1) + 'px');
        })
        .detach()
        .prependTo(self.slogans);
      self.slides
       .children(':last')
        .addClass('ui-banner-prev ui-banner-slides-prev')
        .detach()
        .prependTo(self.slides);
      next();
     })
     .queue("banner", function(next){
      self.rotating = false;
      next();
     });
   }
   self.slides
    .queue("banner", function(next){
     self.sliding = false;
     self._trigger("rotateBack");
     self._trigger("rotate");
     next();
    })
   .dequeue("banner");
  },
  _slideForward: function(step){
   var self = this;
   self.slides.queue("banner", function(next){
    self.sliding = true;
    next();
   });
   for(var a = 1; a <= step; a++){
    self.slides
     .queue("banner", function(next){
      self.slides
       .children('.ui-banner-prev')
        .removeClass('ui-banner-prev ui-banner-slides-prev')
        .css('left', '')
        .detach()
        .appendTo(self.slides);
      self.slogans
       .children('.ui-banner-prev')
        .removeClass('ui-banner-prev ui-banner-slogans-prev')
        .css('marginTop', '')
        .detach()
        .appendTo(self.slogans);
      next();
     })
     .queue("banner", function(next){
      self.slogans
       .children('.ui-banner-current')
        .each(function(i, v){
         var $obj = $(this);
         $obj.animate({
          marginTop: ($obj.outerHeight() * -1) + 'px'
         }, 250, function(){
          $obj.toggleClass('ui-banner-current ui-banner-slogans-current ui-banner-prev ui-banner-slogans-prev');
         });
        })
       .end()
       .children('.ui-banner-next')
        .toggleClass('ui-banner-next ui-banner-slogans-next ui-banner-current ui-banner-slogans-current')
         .next()
          .addClass('ui-banner-next ui-banner-slogans-next');
      self.slides
       .children('.ui-banner-current')
        .each(function(i, v){
         var $obj = $(this);
         $obj.animate({
          left: '-=' + $obj.width() + 'px'
         }, 250, function(){
          $obj.toggleClass('ui-banner-current ui-banner-slides-current ui-banner-prev ui-banner-slides-prev');
         })
        })
       .end()
       .children('.ui-banner-next')
        .animate({ left: '0px' }, 250, function(){
         $(this)
          .toggleClass('ui-banner-next ui-banner-slides-next ui-banner-current ui-banner-slides-current')
          .next()
           .addClass('ui-banner-next ui-banner-slides-next');
         next();
        });
     });
   }
   self.slides
    .queue("banner", function(next){
     self.sliding = false;
     self._trigger("rotateNext");
     self._trigger("rotate");
     next();
    })
    .dequeue("banner");
  },
  _setOption: function(key, value) {
   var self, oldValue = self.options[key];
   switch(key){
    case "className":
     self.element.toggleClass(this.options.className + " " + value);
     break;
    case "interval":
     value = value.toString().split(".")[0].length < 4 ? value * 1000 : value;
     break;
   }
   $.Widget.prototype._setOption.apply(self, arguments); // call the base _setOption method
   self._trigger("setOption", { type: "setOption" }, { // trigger a callback when options change incase the user wants that
    option: key,
    original: oldValue,
    current: value
   });
  },
  destroy: function(){ // undo everything
   var self = this;
   $(document).unbind("keypress.bannerize");
   self.element
    .clearQueue("rotate")
    .unbind("rotate.bannerize")
   .removeClass('ui-banner ' + self.options.className);
   self.slides
    .removeClass('ui-banner-slides')
    .children()
     .removeClass('ui-banner-current ui-banner-slides-current ui-banner-prev ui-banner-slides-prev ui-banner-next ui-banner-slides-next');
   self.slogans
    .removeClass('ui-banner-slogans')
    .undelegate('li', 'click.bannerize')
    .children()
     .removeClass('ui-banner-current ui-banner-slides-current ui-banner-prev ui-banner-slides-prev ui-banner-next ui-banner-slides-next');
   self.arrows.remove();
   self.overlay.remove();
   $.Widget.prototype.destroy.call(self);
  }
 });
})(jQuery);