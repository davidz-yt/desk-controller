#NoEnv  ; Recommended for performance and compatibility with future AutoHotkey releases.
; #Warn  ; Enable warnings to assist with detecting common errors.
SendMode Input  ; Recommended for new scripts due to its superior speed and reliability.
SetWorkingDir %A_ScriptDir%  ; Ensures a consistent starting directory.

; Note the following hotkey prefixes (^ Ctrl) (! Alt) (+ Shift)
: HID Remapper from https://www.jfedor.org/hid-remapper-config/

; ############# Quick Launchers #############

Shift & F15::
; Add necessary controls for mute mic button here
return

Alt & F19::
Run, curl -X POST -H "Authorization: Bearer HomeAssistantAPIKey" -H "Content-Type: application/json" http://192.168.1.3:8123/api/services/script/desk_sit,,hide
return

Alt & F20::
Run, curl -X POST -H "Authorization: Bearer HomeAssistantAPIKey" -H "Content-Type: application/json" http://192.168.1.3:8123/api/services/script/desk_stand,,hide
return

Alt & F21::
Run, Explorer.exe
return

Alt & F22::
Process, Exist, chrome.exe 
If Not ErrorLevel
{
	Run, C:\Program Files\Google\Chrome\Application\chrome.exe 
}
Else
{
	WinActivate, ahk_exe chrome.exe
}
return


; ############ Windows Management ###########

;YouTube Fullscreen Automation (this only works with Chrome and the Windowed Addon)
Alt & F15::
Send f
Sleep 50
Send w
Sleep 350
WinMove, A,,420,2160-1700,3000,1700
return

; Expand Window Top to Bottom
Alt & F16::
WinGetPos, X, Y, W, H, A
WinMove, A,,X,2160-1600,W,1600
return

; Resize Window Small
Alt & F17::
WinGetPos, X, Y, W, H, A
WinMove, A,,X+(W/2)-600,Y+(H/2)-500,1200,1000
return

; Default window position
Alt & F18::
WinGetPos, X, Y, W, H, A
WinMove, A,,(3840-2000)/2,2160-1600,2000,1600
return


; Expand left
Shift & F23::
WinGetPos, X, Y, W, H, A
WinMove, A,,X-200,Y,W+200,H
return

; Move left (TODO: Ctrl+Shift+F23)
Ctrl & F23::
WinGetPos, X, Y, W, H, A
WinMove, A,,X-200,Y,W,H
return

; Compress left
Shift & F24::
WinGetPos, X, Y, W, H, A
WinMove, A,,X+200,Y,W-200,H
return

; Expand right
Alt & F14::
WinGetPos, X, Y, W, H, A
WinMove, A,,X,Y,W+200,H
return

; Move right(TODO: Ctrl+Alt+F14)
Ctrl & F14::
WinGetPos, X, Y, W, H, A
WinMove, A,,X+200,Y,W,H
return

; Compress right
Alt & F13::
WinGetPos, X, Y, W, H, A
WinMove, A,,X,Y,W-200,H
return


; ###### Switch HDMI Input shortcuts ######

Shift & F19::
Run, curl -X POST -H "Authorization: Bearer HomeAssistantAPIKey" -H "Content-Type: application/json" http://192.168.1.3:8123/api/services/script/quick_pc,,hide
return

Shift & F20::
Run, curl -X POST -H "Authorization: Bearer HomeAssistantAPIKey" -H "Content-Type: application/json" http://192.168.1.3:8123/api/services/script/quick_pc2,,hide
return

Shift & F21::
Run, curl -X POST -H "Authorization: Bearer HomeAssistantAPIKey" -H "Content-Type: application/json" http://192.168.1.3:8123/api/services/script/quick_ps5,,hide
return

Shift & F22::
Run, curl -X POST -H "Authorization: Bearer HomeAssistantAPIKey" -H "Content-Type: application/json" http://192.168.1.3:8123/api/services/script/quick_hdmi4,,hide
return


; ###### Switch USB Input shortcuts ######
; NOTE: This needs to go after the respective similar HDMI shortcuts for the "Ctrl" modifier to work.

Ctrl & F19::
Run, curl -X POST -H "Authorization: Bearer HomeAssistantAPIKey" -H "Content-Type: application/json" http://192.168.1.3:8123/api/services/script/usb1,,hide
return

Ctrl & F20::
Run, curl -X POST -H "Authorization: Bearer HomeAssistantAPIKey" -H "Content-Type: application/json" http://192.168.1.3:8123/api/services/script/usb2,,hide
return

Ctrl & F21::
Run, curl -X POST -H "Authorization: Bearer HomeAssistantAPIKey" -H "Content-Type: application/json" http://192.168.1.3:8123/api/services/script/usb3,,hide
return

Ctrl & F22::
Run, curl -X POST -H "Authorization: Bearer HomeAssistantAPIKey" -H "Content-Type: application/json" http://192.168.1.3:8123/api/services/script/usb4,,hide
return


; ###### Light and Screen Power shortcuts ######

Shift & F18::
Run curl -X POST -H "Authorization: Bearer HomeAssistantAPIKey" -H "Content-Type: application/json" http://192.168.1.3:8123/api/services/script/monitor_light_toggle,,hide
return

Shift & F14::
Run curl -X POST -H "Authorization: Bearer HomeAssistantAPIKey" -H "Content-Type: application/json" http://192.168.1.3:8123/api/services/script/lightbar_toggle,,hide
return

Shift & F17::
screenToggle := !screenToggle
if (screenToggle)
{
	Run, curl -X POST -H "Authorization: Bearer HomeAssistantAPIKey" -H "Content-Type: application/json" http://192.168.1.3:8123/api/services/script/lg_screen_off,,hide
} else {
	Run, curl -X POST -H "Authorization: Bearer HomeAssistantAPIKey" -H "Content-Type: application/json" http://192.168.1.3:8123/api/services/script/lg_screen_on,,hide
}
return

Shift & F13::
Run, curl -X POST -H "Authorization: Bearer HomeAssistantAPIKey" -H "Content-Type: application/json" http://192.168.1.3:8123/api/services/script/toggle_power_lgtv,,hide
return