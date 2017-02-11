Access_token="ya29.GlvrA6_KFdmiE2UKZ7mt33PWXZ6e27oqrSUm32CLZYVkUeCtEQ8dQ8Xn4C-uj4uGe0tNM2EwLOp2diuQbjR668_8u8nHtka3kYG3DdeTK2RTrDD-cKtL2x2uLLEh";

# https://developers.google.com/google-apps/calendar/v3/reference/

#curl "https://www.googleapis.com/youtube/v3/channels?part=contentDetails&mine=true&access_token=${Access_token}"

#curl "https://www.googleapis.com/youtube/v3/subscriptions?part=snippet&mySubscribers=true&access_token=${Access_token}"

#curl "https://www.googleapis.com/youtube/v3/playlists?part=contentDetails&mine=true&access_token=${Access_token}"

curl "https://www.googleapis.com/youtube/v3/playlistItems?part=id&playlistId=HL&access_token=${Access_token}"
