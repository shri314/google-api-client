<?php
    $token_url     = 'https://accounts.google.com/o/oauth2/token';

    $post_data = array(
                        'client_id'     => '199281123173-s69303ojtuegf7n5pf178kil33c53v1i.apps.googleusercontent.com',
                        'client_secret' => 'oFQK3ELzm64e-D5BdecUL-j_',
                        'refresh_token' => '1/VeLhLTdlK464oqhjhr5vUHVMIL_HbpQn0BrYAtc3EiU',
                        'grant_type'    => 'refresh_token'
                     );

    $ch = curl_init();

    curl_setopt($ch, CURLOPT_URL, $token_url);
    curl_setopt($ch, CURLOPT_POST, 1);
    curl_setopt($ch, CURLOPT_POSTFIELDS, $post_data);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);

    $result = curl_exec($ch);

    print_r( $result );
?>
