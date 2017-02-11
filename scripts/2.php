<?php
$url = 'https://accounts.google.com/o/oauth2/token';

$post_data = array(
                    'client_id'     =>   '199281123173-s69303ojtuegf7n5pf178kil33c53v1i.apps.googleusercontent.com',
                    'client_secret' =>   'oFQK3ELzm64e-D5BdecUL-j_',
                    'code'          =>   '4/KoW2ToiUqIpXEyAkyYIm9vLMGQsCAW_6JgxDpuV90fM',
                    'redirect_uri'  =>   'urn:ietf:wg:oauth:2.0:oob',
                    'grant_type'    =>   'authorization_code'
                  );

$ch = curl_init();

curl_setopt($ch, CURLOPT_URL, $url);
curl_setopt($ch, CURLOPT_POST, 1);
curl_setopt($ch, CURLOPT_POSTFIELDS, $post_data);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);

$result = curl_exec($ch);

print_r( $result );

$token = json_decode($result);

echo $token->refresh_token . "\n";
?>
