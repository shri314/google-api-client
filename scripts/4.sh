CalendarID="FILL"
Access_token="FILL";

# https://developers.google.com/google-apps/calendar/v3/reference/

cat Calendar.csv | sed -e '1d' | head -3 | while read -r line
                                           do
                                              START=$(echo $line | sed -e 's/,.*//');
                                              END="$(date +%F -d "$START 1 day")"
                                              TITLE=$(echo $line | sed -e 's/^..........,//');

                                              echo "----------------------------------------------------"
                                              echo START=$START;
                                              echo END=$END;
                                              echo TITLE=$TITLE;

                                              sleep 1;
                                              curl "https://www.googleapis.com/calendar/v3/calendars/${CalendarID}/events?access_token=${Access_token}" \
                                                    -H "Content-Type: application/json" \
                                                    -d '{
                                                          "summary": "[EVENT] - '"$TITLE"'",
                                                          "start": { "date": "'"$START"'", "timeZone": "Asia/Kolkata" },
                                                          "end": { "date": "'"$END"'", "timeZone": "Asia/Kolkata" },
                                                          "recurrence": [ "RRULE:FREQ=YEARLY" ],
                                                          "reminders": {
                                                             "overrides": [
                                                                {
                                                                   "method": "popup",
                                                                   "minutes": 10
                                                                },
                                                                {
                                                                   "method": "popup",
                                                                   "minutes": 120
                                                                },
                                                                {
                                                                   "method": "popup",
                                                                   "minutes": 780
                                                                }
                                                             ],
                                                             "useDefault": false
                                                          },
                                                          "transparency": "transparent"
                                                        }'
                                           done

