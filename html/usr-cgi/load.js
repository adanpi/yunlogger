<script type="text/javascript">

function load()
{
  tmp = findSWF("ofc");
    x = tmp.load('{"title":{"text":"hello"}, "x_axis":{"max":6},"elements":[{"type":"line","text":"moo","font-size":10,"values":[5,8,9,4,7,8]}]}');
    }
    
    function onrollout2()
    {
      tmp = findSWF("ofc");
        x = tmp.rollout();
        }
        
        function findSWF(movieName) {
          if (navigator.appName.indexOf("Microsoft")!= -1) {
              return window[movieName];
                } else {
                    return document[movieName];
                      }
                      }
                      
                      </script>
                      
