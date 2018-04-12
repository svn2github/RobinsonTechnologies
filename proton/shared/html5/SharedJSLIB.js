// here you write JS "handlers"
mergeInto(LibraryManager.library, 
{

JLIB_EnterString: function(message, defaultText)
{
 var pMessage = Pointer_stringify(message);
  var pURL = Pointer_stringify(defaultText);
	var person = prompt(pMessage, pURL);
	
	if (person == null) return 0;
	
	 console.log("Got "+person);
	 
	 // Create a pointer using the 'Glue' method and the String value
    var ptr  = allocate(intArrayFromString(person), 'i8', ALLOC_NORMAL);
    return ptr;
},


JLIB_Test: function(URLStr) 
  {
   var url = Pointer_stringify(URLStr);
  
    var OpenPopup = function() 
    {
      //run the thing we're supposed to
      console.log("Doing OpenPopUp");
	
      window.open(url, "_blank");
      //unhook this, we're done
      var el = document.getElementById('canvas');
      if (el)
      {
	    el.removeEventListener('click', OpenPopup)
	    el.removeEventListener('touchend', OpenPopup)
	  } 
    };
    
    var el = document.getElementById('canvas');
    if (el)
    {
	    el.addEventListener('click', OpenPopup, false);
	    el.addEventListener('touchend', OpenPopup, false);
	  } else
	  {
	     console.log("Can't find a canvas element. Is it called #canvas instead or something?  Check RTJavaUtils.jslib.  Calling without popup safety.");
	     window.open(url);
	  }
   
  },
});