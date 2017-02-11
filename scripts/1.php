<?php

$params = array(
                    'client_id'     =>   '199281123173-s69303ojtuegf7n5pf178kil33c53v1i.apps.googleusercontent.com',
                    'redirect_uri'  =>   'urn:ietf:wg:oauth:2.0:oob',
                    'scope'         =>   'https://www.googleapis.com/auth/youtube',
                    'response_type' =>   'code'
               );

$url = 'https://accounts.google.com/o/oauth2/auth?' . http_build_query($params);        
echo $url."\n";
?>
