# date-rfc
Header only C++ library for parsing dates defined by rfc specifications:
 - RFC 1123 (example: Tue, 31 Dec 2010 23:59:59 GMT)
 - RFC 3339 (example: 1985-04-12T23:20:50.52Z)

## Installing
It is header only and self-sufficient library, so just copy and use it:
```
git clone https://github.com/Koheiru/date-rfc.git
```

## RFC 1123 specification
Specification: https://tools.ietf.org/html/rfc1123#page-55

Example: Tue, 31 Dec 2010 23:59:59 GMT

Format:

     date-time   =  [ day "," ] date time        ; dd mm yy
                                                 ;  hh:mm:ss zzz
     
     day         =  "Mon"  / "Tue" /  "Wed"  / "Thu"
                 /  "Fri"  / "Sat" /  "Sun"
     
     date        =  1*2DIGIT month 2*4DIGIT      ; day month year
                                                 ;  e.g. 20 Jun 2012
     
     month       =  "Jan"  /  "Feb" /  "Mar"  /  "Apr"
                 /  "May"  /  "Jun" /  "Jul"  /  "Aug"
                 /  "Sep"  /  "Oct" /  "Nov"  /  "Dec"
     
     time        =  hour zone                    ; ANSI and Military
     
     hour        =  2DIGIT ":" 2DIGIT [":" 2DIGIT]
                                                 ; 00:00:00 - 23:59:59
     
     zone        =  "UT"  / "GMT"                ; Universal Time
                                                 ; North American : UT
                 /  "EST" / "EDT"                ;  Eastern:  - 5/ - 4
                 /  "CST" / "CDT"                ;  Central:  - 6/ - 5
                 /  "MST" / "MDT"                ;  Mountain: - 7/ - 6
                 /  "PST" / "PDT"                ;  Pacific:  - 8/ - 7
                 /  1ALPHA                       ; Military: Z = UT;
                                                 ;  A:-1; (J not used)
                                                 ;  M:-12; N:+1; Y:+12
                 / ( ("+" / "-") 4DIGIT )        ; Local differential
                                                 ;  hours+min. (HHMM)

Remarks:
 - All mail software SHOULD use 4-digit years in dates, to ease the transition to the next century.

## RFC 3339 specification (in progress)
Specification: https://tools.ietf.org/html/rfc3339

Example: 1985-04-12T23:20:50.52Z

Format:

     date-fullyear   = 4DIGIT
     date-month      = 2DIGIT  ; 01-12
     date-mday       = 2DIGIT  ; 01-28, 01-29, 01-30, 01-31 based on
                               ; month/year
     time-hour       = 2DIGIT  ; 00-23
     time-minute     = 2DIGIT  ; 00-59
     time-second     = 2DIGIT  ; 00-58, 00-59, 00-60 based on leap second
                               ; rules
     time-secfrac    = "." 1*DIGIT
     time-numoffset  = ("+" / "-") time-hour ":" time-minute
     time-offset     = "Z" / time-numoffset

     partial-time    = time-hour ":" time-minute ":" time-second
                       [time-secfrac]
     full-date       = date-fullyear "-" date-month "-" date-mday
     full-time       = partial-time time-offset

     date-time       = full-date "T" full-time



## License
This project is licensed under the MIT License - see the LICENSE.txt file for details.