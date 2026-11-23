# Changelog

RESONIK edition: <https://resonik.mikroutik.com> · Upstream user manual: <https://esp32-si4732.github.io/ats-mini/manual.html> · Flashing instructions: <https://esp32-si4732.github.io/ats-mini/flash.html>

<!-- towncrier release notes start -->

## 2.46-r5 (2026-11-24)

RESONIK edition, Phase 5: digital modes decoded on the radio itself (RTTY, NAVTEX, PSK31 and weather fax) and a Decodes tab in Web Control, Band View, Freq Entry, a setup wizard and a new version notice, with the latest ats-mini. From R4: Menu → System → Update FW over Wi-Fi, or the Install button on https://resonik.mikroutik.com. Settings and memories are kept.


### Added

- **BT Power** (Menu → System, last page, Web Control → Settings → Power and time, and the Config web page): the Bluetooth transmit power, from -12 to 9 dBm (0 dBm as delivered). Lower it when the phone or computer is close, raise it when the connection drops at a distance. It is used while Bluetooth is on and kept while it is off.
- **Backup and restore** on the radio's Config web page: download all settings, bands, memories and Wi-Fi networks as one file, and restore it later or on another radio running the same settings layout. The radio restarts after a restore.
- **Band View** (**Menu → Tuning → Band View**): 160 frequencies around the one in use, swept again and again. The sweep in progress is shown as bars that fill in as it goes, the finished sweeps as a waterfall under it, weak blue to strong yellow, with the frequencies the view starts and ends at. Turn to move the cursor, click to tune there. In Web Control the **Band View** button of the Band scope box starts it and shows it in the browser; a click on a peak tunes there. **Auto-Store** (**Menu → Memory → Auto-Store**): one sweep around the frequency, then the strongest stations go into free memory slots, named "Auto", skipping those already stored.
- **Battery** (**Menu → Display → Battery**): what the battery shows beside its icon: volts alternating with the time left (as before), volts only, percent, or the icon alone.
- **Capacitor** (Menu → Tuning, and Web Control → Settings → Receiver): the automatic antenna capacitor for the MW1, MW2, MW3 and 160 m bands, until now only on the Config web page. Off by default; it only works on a radio without a Hi-Z input circuit. From ats-mini ([discussion #409](https://github.com/esp32-si4732/ats-mini/discussions/409)).
- **Decoder Audio** (**Menu → Ham & SWL → Dec. Audio**, and the **Decodes** tab of Web Control): how much of the radio's sound the FT8/FT4 and Digital pages take in. **3 kHz fast** (the default) holds everything these modes use at half the work, so the radio and Web Control stay quick; **6 kHz full** is the input of earlier versions, at twice the work. Changed from Web Control it applies at once.
- **Decodes** tab in Web Control: while the FT8/FT4, CW or Digital page is open on the radio, the browser shows the band as a waterfall, the decoded messages with the station's country and flag, a map of the stations heard (with lines from your locator), and the CW, RTTY, NAVTEX or PSK31 text. A **DX alarm** sounds when a chosen callsign or country is heard, and **Save log** downloads the decodes as a text file.
- **Digital** (**Menu → Ham & SWL → Digital**): RTTY (amateur 45.45 baud / 170 Hz and DWD weather 50 baud / 450 Hz), NAVTEX and PSK31, decoded on the radio with a spectrum of the band. The header shows the frequency the radio is tuned to. Each mode keeps its own frequency (at first: RTTY 14080, RTTY DWD 10099.3, PSK31 14070, WEFAX 8038.1 kHz), so going from NAVTEX to RTTY leaves medium wave at once; the page never works on FM. The decoder finds the signal by itself; RTTY reads normal and reversed stations alike. Click for the next mode, turn to tune, press to leave. Needs the V4B board, as the FT8 page.
- **Enc. Accel.** (Menu → System, last page, and Web Control → Settings → Power and time): switch off the speed-up of fast knob turns for one step per click. On by default. From ats-mini ([#378](https://github.com/esp32-si4732/ats-mini/discussions/378)).
- **Freq Entry** (**Menu → Tuning → Freq Entry**): a number pad to type a frequency with the knob, then **kHz** or **MHz**. The radio changes band (and mode, from FM) when the frequency is outside the current band.
- **Log** page in the radio's web portal (the menu button was **Log (CSV)**, a download only): choose the signal logger's interval or switch it off, see how full the file is and its latest 50 lines, newest first under a line for each day, then **Download (CSV)** or **Delete log** (after a question). Menu → Memory → Logger on the radio and the setting in Web Control work as before.
- **New version notice** (**Menu → System → Upd. Notice**, on by default): with Wi-Fi connected, the radio looks for a newer RESONIK release a few minutes after connecting and then once a day, and shows a page with **Update now**, **Later** or **Skip this version**. An arrow beside the Wi-Fi icon and a banner on the radio's web pages show that an update is waiting.
- **Setup wizard** at first start (and after a factory reset): time zone, knob type (20 or 30 clicks), band plan for your licence, then Wi-Fi. A short press skips it. Radios updated from an earlier version keep their settings and do not see it.
- **WEFAX** (radiofax) on the Digital page: weather charts at 120 lines a minute, drawn on the radio and, larger, in the Decodes tab of Web Control, where **Save picture** downloads it. Tune 1.9 kHz below the published frequency in USB.
- **Web Control → Decodes**: RTTY, RTTY DWD, PSK31 and WEFAX can be started on a frequency from a list (the DWD and Northwood frequencies, the RTTY and PSK31 parts of the bands) or **As tuned**. While CW or a Digital mode runs, **◀ ▶** turn the radio's knob (tune, or the next NAVTEX channel) and **Tune** goes to a typed frequency, staying in USB; the row shows the radio's frequency. A frequency typed on the Receiver tab while the Digital page is open no longer changes the band's mode, and the band plan no longer switches the mode under the Digital page while tuning.
- Add Wi-Fi and Bluetooth transmit power settings to the web interface.
- Digital page: the **whole decoded text** of RTTY, NAVTEX and PSK31 is kept, about 16 000 characters with the line breaks as sent. **Push the knob and turn** reads it back on the radio, seven lines at a time (turn scrolls, a click returns to the live text). Web Control shows it in a box that scrolls and **Save text** downloads it; the browser fetches what it has not got yet, so text decoded before the page was opened is there too. Each start of a decoder that receives something writes a head line with the mode, the frequency and the UTC time.
- English manual (PDF): the R5 chapters. Setup at first start; Freq Entry, Band View and Auto-Store; RTTY, NAVTEX, PSK31 and WEFAX; Decodes in the browser with Decoder audio; **Signals to try the decoders with** (where to tune for FT8, FT4, CW, RTTY, NAVTEX, PSK31 and WEFAX, and what to expect); update notice, backup and the Status page; and R5 notes in the Battery, Beacons, FT8, CPU and Web Control chapters. The web manual gains Freq Entry, the battery display, the Status page, starting a decoder from the browser and the clock.
- Manual: **Signals to try the decoders with**, a table of stations and frequencies to check FT8, FT4, CW, RTTY, NAVTEX, PSK31 and WEFAX with, how to tune each and what to expect; and how to see the setup wizard again.
- Manual: full sections for the two new settings, **Enc. Accel.** (what the knob does with it on and off, when to choose which) and **Capacitor** (what it is, the bands it acts on, which radios it helps and how to try it), with their screens; the English manual's table of menu groups lists every R5 item.
- Manuals (English and Persian PDF, and the web manual): an appendix, **Learning the digital modes**. For FT8 and FT4, CW, RTTY, RTTY DWD, NAVTEX, PSK31 and WEFAX it says what the mode is and who sends it, what it looks like in the waterfall, how to tune it and how to read what the decoder writes, with an example message and an exercise; with the basics (frequency and tone, baud and shift, dB, UTC, locators), a suggested order and a glossary.
- NAVTEX: **eleven channels** in place of 518 kHz alone: 518, 490 and 4209.5 kHz and the eight short-wave channels for safety information on the high seas (4210 to 26100.5 kHz). On the radio the knob steps through them and the page tunes each by itself (the header shows the channel and USB, as the other modes show their frequency and USB); in Web Control they are the second list of the decoder row. **sync** shows when the decoder is locked to a station, and the browser's waterfall marks the tones the decoder listens to.
- Persian manual (PDF): the R5 chapters, the same as in the English manual, with the two new settings (Capacitor, Enc. Accel.) and the signals to try the decoders with. Menu paths run right to left (Menu ← Ham & SWL ← Band Plan) and the sign-off reads "73 de resonik.mikroutik.com".
- Status web page: after a restart by the watchdog or a crash, **Last restart** also says what the radio was doing just before (the last steps and the tasks running), to copy into a report.
- The Status web page and the About screen name the commit a build was made from, after the date: "v2.46-r5, built 22 Nov 2026 (0d9c9f3)". The seven characters are the same as at the end of the build's file name, so a radio can be matched with its build.
- The radio's Status web page also shows how long the radio has been running, why it last restarted (switched on, update, crash, watchdog, low voltage), the processor's temperature and speed, free memory and the Wi-Fi signal. Each figure says whether it is good, fair or bad, in green, yellow or red.
- Web Control shows the radio's clock beside the station name: UTC and, when the radio's time zone is not UTC, local time; "Clock not set" until the radio has the time.
- Web Control, Beacons tab: **Follow a beacon** at the top, with the list of the 18 beacons and **Start** and **Stop** buttons. The line beside them says which beacon is being followed, the frequency it is on now or the seconds to its next turn, and why following did not start or ended. Before, following could only be started by tapping the map and stopped from the radio's menu.
- Web Control, Decodes tab: **Decoder on the radio**, a row to start and stop the decoders from the browser: FT8 and FT4 with a list of their bands, CW, RTTY, RTTY DWD, NAVTEX, PSK31 and WEFAX. **Stop** closes the decoder and the radio goes back to where it was. The line beside the buttons says what is running. Before, a decoder had to be opened on the radio.
- Web Control, Decodes tab: **Save audio (WAV)** while a Digital mode runs: the sound the decoder heard, the last minute of it since the last change of mode or channel, to send with a report when a station is heard but not decoded.
- Web Control, Decodes: a **NAVTEX timetable** for 518, 490 and 4209.5 kHz: who is on the air now and who comes next, the distance from My locator, nearest first or within a range; a click on a line starts NAVTEX on that channel. 238 stations, checked on 22 Nov 2026 against Alan Gale G4TMV's World NAVTEX Database for DXers, Wikipedia's list and fldigi's station file.


### Changed

- Beacons: while the followed beacon is silent (130 of every 180 seconds) the radio waits on 14.100 MHz in CW, where its next run begins, and goes there as soon as following starts. Starting is seen and heard at once, and the first of the beacon's five slots is no longer cut short by the band change. Ending the follow with **Off** or **Names only** in the list, or **Stop** in Web Control, takes the radio back to the band, frequency and mode it was on before; tuning away or choosing another band ends it where you are. Turning through the list on the radio changes band only for the name the knob rests on.
- CAT control now uses the Kenwood TS-480 support of ats-mini itself, and is offered on three ports: **USB Port**, **TCP Port** (Wi-Fi, port 60000) and **Bluetooth**, each with the choice **CAT TS-480** (it was **CAT**, on USB only). New with it: VFO A and B, squelch, tuning steps up and down, automatic status reports (`AI`), and an S-meter on the scale Hamlib expects. Kept from RESONIK: the CW mode (`MD3`), data modes answered on USB, and the fixed answers flrig, OmniRig and logging programs ask for. A CAT program asking for the frequency every second does not stop a seek or a scan. A radio that had Bluetooth on **HID** keeps HID after the update.
- Status web page: the **Firmware** row no longer repeats itself with "F/W:" and gives the version as the page header and the release names do, with the day it was built: `v2.46-r5, built 21 Nov 2026`.
- The **Ham & SWL** tile of the menu shows the international amateur radio symbol in yellow: aerial, coil and earth, drawn from the symbol itself, with its two straight wires shortened so that the rest is larger. It stands at the side of the tile with the name beside it. The tile had shared the mast of the Beacons tile.
- The **Scanner** tile (Menu → Tuning) is now called **Auto Scan**: it was taken for **Scan**, the band graph, which sits next to it. Its status line says Auto Scan too. While it moves, the name of the station it left no longer stays on the screen.
- Web Control answers faster: while a browser has it open the radio runs its processor at full speed (240 MHz, a little more battery; the **CPU** setting applies again when the browser leaves, and the setting on the page says so; the Status page shows the speed that is set), and after a button on the page the frequency panel and the radio's screen are sent at once instead of at the next regular update.
- Web Control: the page's short notes (seek results, beacon follow) appear at the bottom of the window wherever the page is scrolled. They used to sit in the frequency panel at the top, out of sight from the lower tabs.
- Web pages: every page of the radio's web portal, **Web Control included**, has the same top: the RESONIK bar with the firmware version, then the row of all eight pages (Status, Control, Memory, Config, Wi-Fi, Update, Patches, Log) with the page you are on lit. A footer on every page links to **resonik.mikroutik.com**, the **user manual** and **GitHub**. The memory figures at the foot of each page moved to the Status page; the answer to Restore is shown in the same frame. The frame stays in the same place on every page: a page short enough to need no scrollbar no longer moves it sideways, and the Config and Patches pages fit narrow phones (their file fields made them wider than the screen).
- Wi-Fi **Power** on the radio is set in dBm, from 2 to 20 dBm (17 dBm as before), the same list as in Web Control and on the Config web page, where the Bluetooth LE power is set too. A radio set to Low or Lowest keeps 13 or 9 dBm.


### Fixed

- A program connected over Wi-Fi (**TCP Port**: CAT TS-480 or Ad hoc) keeps Wi-Fi on while the FT8, CW Decoder and Digital pages are open, as an open Web Control page does. Before, those pages switched Wi-Fi off and the program lost the radio.
- Beacons: following a beacon could not be left while the radio was waiting for the beacon's turn. Switching to FM or another band played for a moment, then the radio went back to the beacon. Tuning away, another band or FM now ends the follow at any time.
- Fix Bluetooth HID seeking sometimes stopping immediately on the same frequency.
- Fix crashes when rapidly refreshing the web configuration pages.
- Web Control Seek: the radio's screen and the page now follow the search together. In AM and FM (11 m CB, broadcast bands) the page showed nothing while the radio searched; in SSB and CW (ham bands) the page showed the frequencies tried while the radio's screen stayed still. After a seek that found nothing, the page also showed the last frequency tried instead of the radio's. A long Seek from the page could also stop the radio's web pages answering until a restart; the page now gets the search four times a second at most.
- Web Control keeps its connection healthier: the radio no longer piles messages into a connection that cannot take them, closes a browser connection that has been stuck for 5 seconds so it reconnects cleanly, and the page reconnects by itself (showing "reconnecting") when it hears nothing from the radio for 8 seconds. A browser that drops off for a moment keeps Wi-Fi on for a minute on the FT8, CW and Digital pages, and decodes are sent only while the Decodes tab is shown. On the FT8, CW and Digital pages the radio's screen goes to the browser twice a second, so the browser stays quick while the radio decodes.

## 2.46-r4 (2026-11-20)

RESONIK edition R4.1: R4 brought up to upstream ats-mini v2.46, with a fix for the CW decoder page. From R4: Menu → System → Update FW over Wi-Fi, or the Install button on https://resonik.mikroutik.com. Settings and memories are kept.


### Changed

- The CW decoder page plays at least at the FT8 page's level and never quieter than your own volume, so the sound no longer drops when the page opens.


### Fixed

- Mute and squelch stay as they were across band and mode changes; before, a band or mode change could let sound through while the screen still showed Muted or the squelch closed (from upstream ats-mini v2.46).

## 2.45-r4 (2026-11-15)

RESONIK edition, Phase 4: control the radio from a phone or computer, and set up Wi-Fi on the radio itself. From R3: Menu → System → Update FW over Wi-Fi, or the Install button on https://resonik.mikroutik.com. Settings and memories are kept.

### Added

- **Web Control** (`http://<radio-ip>/control`, or the **Control** link on the radio's web pages): the whole radio in a browser on a phone, tablet or computer, on the same Wi-Fi.
  - **Receiver tab:** large frequency, band, mode, step, filter, S-meter, battery (volts and percent), the memory you are on, RDS station name and radio text; tuning arrows, Seek, frequency box (kHz; MHz on FM), mode buttons with Stereo/Mono shown on FM, Band, Step, Filter and AGC/attenuator lists, volume slider, one button per band.
  - **Band scope:** Sweep measures 41 points around the frequency (FM ±2 MHz, AM ±100 kHz, SSB/CW ±20 kHz); click a bar to tune there.
  - **Memory tab:** tune to a memory, save the current station, delete one.
  - **Beacons tab:** a world map of the 18 NCDXF/IARU beacons. The five on the air pulse in their band's colour with a countdown; tap a frequency to tune there in CW and the radio colours each beacon it hears (well, weakly, not heard), slot by slot. Tap a station to follow it across the bands.
  - **Settings tab:** every setting of the radio's menus, in groups, applied and saved at once. Bluetooth, USB and Wi-Fi modes and firmware updates stay on the radio.
  - **More tab:** the radio's own screen with its knob and buttons (sent only while shown), and **Listen** (experimental): the radio's sound in the browser on V4B boards, phone quality, with a hiss filter and a choice of how quiet the radio's speaker plays.
- **Wi-Fi on the radio** (**Menu → System → Wi-Fi**): one page for everything Wi-Fi. The top shows where to find the radio's web pages (its IP address, or 10.1.1.1 on its own access point). **Mode**, **Power**, and **Join a network**: the networks in range with signal, security and channel, the password typed with the knob on an on-screen keyboard (letters, capitals, symbols, show or hide). The network is saved and the radio connects at once. The radio has 2.4 GHz Wi-Fi only.
- **Seek in SSB and CW**, on the radio and in the browser: the receiver chip only seeks in AM and FM, and Seek used to move one tuning step in SSB and CW. It now searches 1 kHz at a time with the sound muted and stops on the first signal, at most 200 kHz per press, and says so when it finds nothing.
- **Beacons: Off and Names only.** Off now also hides the beacon names on the beacon frequencies; Names only shows the beacon due in each slot without following one. Saved across restarts.
- **Dark theme** on all the radio's web pages, with the RESONIK logo and the menu as buttons.

### Changed

- **The classic menu has the same six groups as the tiles** (Tuning, Audio, Memory, Ham & SWL, Display, System), each with its items and a Back row; a setting goes back to its group when done ([#4](https://github.com/ep3mhb/resonik-ats-mini/issues/4)).
- Wi-Fi, Wi-Fi Power and joining a network are one **Wi-Fi** item. **Performance** is now **CPU**, with the speeds 80, 160 and 240 MHz.
- **Faster band and mode changes** between SSB bands (about 40 ms instead of 260 ms).
- The CW decoder page and Listen use a steady receiver level, so decoding and the web sound do not depend on the volume setting.
- RDS options shortened (ALL-CT EU, ALL EU...) to fit the classic menu.

### Fixed

- A radio on a beta could not update to the release with the same number over Wi-Fi ("Already up to date") ([#3](https://github.com/ep3mhb/resonik-ats-mini/issues/3)). Builds from now on mark betas; a radio still on v2.45-r3-beta1 updates once by uploading the release file at `http://<radio-ip>/update`.
- Sleep Mode in the classic menu listed its options twice.
- The FT8 and CW pages say "unplug headphones" when there is no audio: on V4B boards, headphones in the radio cut the line the decoders listen to.


## 2.45-r3 (2026-11-13)

RESONIK edition, Phase 3. From R2: Menu → System → Update FW over Wi-Fi, or the Install button on https://resonik.mikroutik.com. Settings and memories are kept. Thanks to everyone who tested the beta.

### Added

- **FT8 decoder** (**Menu → Ham & SWL → FT8**): a full-screen page that decodes FT8 from the radio's own audio, with a waterfall and the messages of each 15 second slot (signal report, audio frequency, text; CQ calls in green). It tunes the FT8 frequency of the band in use (or 40 m) in USB 3.0k; the knob steps through 160, 80, 40, 30, 20, 17, 15, 12 and 10 m. With the clock set (time sync over Wi-Fi) it decodes from the first slot; **without a clock it still decodes**: it finds the band's slot timing from the stations it hears, usually within a minute ("No clock: syncing", then "Synced to band" in the header). The page pauses Wi-Fi and silences the speaker while it is open, and leaving puts the radio back exactly as it was. Needs the V4B board, which wires the speaker line to GPIO11. The speaker is silenced while the page is open, unless the board takes its audio after the speaker amplifier: then the page leaves the amplifier on ("Spk" next to the input level). Uses ft8_lib (Karlis Goba, MIT) and KISS FFT (Mark Borgerding, BSD-3), following the epiHATR fork, which first decoded FT8 on this radio.
- **FT4** on the same page (**Menu → Ham & SWL → FT8/FT4**): turning the knob past 10 m goes on to the FT4 frequencies of 80 to 10 m, with 7.5 second slots. FT4 is shown in yellow in the header.
- **Frequency lock** (**Menu → Tuning → Freq Lock**): the knob no longer tunes; it sets the volume instead, and press-and-turn digit tuning is off. A padlock after the VFO box shows it is on. Saved across restarts.
- **Wi-Fi Power** (**Menu → System → Wi-Fi Power**): Normal (17 dBm), Low (13 dBm) or Lowest (8.5 dBm). Lower power helps boards that reset when Wi-Fi transmits and saves some battery, at the cost of range.
- **Performance** (**Menu → System → Performance**): processor speed Battery (80 MHz, as before), Balanced (160 MHz) or Fast (240 MHz). Faster makes the screen, menus and tuning feel quicker and costs some battery time; the battery estimate shows the difference.
- **CW decoder** (**Menu → Ham & SWL → CW Decoder**): Morse to text on a full-screen page, with speed, tone, level and key indicators; **CW Strip** shows the decoded text on the normal screen in CW mode.
- **Band scanner** (**Menu → Tuning → Scanner**): steps through the band and holds on a signal. In SSB and CW it moves on 4 seconds after the signal goes; on broadcast stations (AM, FM), which never go quiet, it stays 5 seconds on each, and in FM it skips the next 200 kHz so a strong station does not stop it twice. **Turning the knob sets the direction** (right up, left down); a click stops it.
- **Band Plan → Auto**: tuning into a segment of an amateur band sets mode, filter and step for it (CW, digital, beacons, phone). In AM it switches when you tune into an amateur band from outside; AM chosen by hand inside the band stays ([#2](https://github.com/ep3mhb/resonik-ats-mini/issues/2)). **Plan Area** selects **All regions**, **Region 1**, **Region 2**, **Region 3**, **Iran** (no 160, 80 or 30 m, after the national frequency table) or **UAE** (160 m only 1810–1850 kHz); parts not allowed are marked on the scale. All regions shows every region's bands (for example 80 m to 4000 kHz and 40 m to 7300 kHz), with the Region 1 details where Region 1 has the band. The Region 2 and 3 plans are simplified guidance: check your own licence.

### Fixed

- **No more freezes after Wi-Fi connects.** The time from a web server and the propagation data are fetched in the background; on slow or blocked networks they used to stop the knob, menu and screen for 8 to 36 seconds, again after every return from the FT8 and CW pages. The time is not asked for again once the clock is set.
- The radio could restart when the FT8 or CW page was opened during such a download (Wi-Fi was switched off under it). The pages now wait for it to finish.
- FT8 audio is sampled at exactly 12 kHz (it was 12048 Hz, about 2 dB less sensitive).
- The CW decoder no longer prints rows of "E" on noise: static crashes and clicks, loud beside the pitch too, are ignored, and so are blips shorter than half a dot.
- Seek stays inside the band (upstream #165, after the mattafaak fork).
- Encoder acceleration eases off one step at a time; smoother SSB tuning; steadier RDS station names (after the jokubasver fork).
- The CW decoder input is disconnected from the ADC while idle, so no faint buzz reaches the speaker.

### Credits

- About → Authors names the forks whose work is in this release: epiHATR (FT8), jokubasver and mattafaak (fixes).


## 2.45-r2 (2026-11-08)

RESONIK edition, Phase 2.

### Added

- **Beacon tracker** for the NCDXF/IARU beacons: on 14100, 18110, 21150, 24930 and 28200 kHz the station line names the beacon on the air now, with seconds left in its slot. **Menu → Beacons** follows one beacon across the five bands as it moves, a quick check of which paths are open.
- **Memory scan** (**Menu → Mem Scan**): steps through the stored memories and holds on any with a signal, resuming 4 seconds after it goes. Uses the mode's squelch setting if one is set. Rotate or click to stop.
- **Signal logger** (**Settings → Logger**: every 10 s, 1 min or 5 min): time, frequency, mode, band, RSSI, SNR and station name to a CSV file, downloadable from the web page (**Log (CSV)**). Clear it with Settings → Logger → Clear log.
- **Propagation** (**Settings → Propagation**): solar flux, sunspots, A and K index and the day and night HF band conditions from hamqsl.com (N0NBH). Fetched while Wi-Fi is up, including the Sync at power-on.
- **CAT control** (**Settings → USB Port → CAT**): Kenwood TS-480 commands for frequency, mode, S-meter and volume, for Hamlib, flrig, WSJT-X (receive), fldigi and logging programs.
- **EiBi schedules** now come from the season's CSV file, with days of the week (a station is only shown on its broadcast days) and language and target area after the name, e.g. `[E > ME]`. Load the schedule again once after updating (Settings → Load EiBi).

### Changed

- **Tile menu redesigned and now the default menu.** The menu opens on six categories: Tuning, Audio, Memory, Ham & SWL, Display and System. Each category is a page of large tiles with a **Back** tile at the top left; everyday items come first, the rest A to Z. FM Region sits under Tuning and FM Stereo under Audio. Solid colours (pink, coral, yellow, green, blue, deep navy), full-colour icons, the Inter SemiBold font, and the selected tile pops out slightly. Click opens or selects, a short press closes the menu from any page. The classic list is still under Display → **Menu Style**. Radios updated from r1 start on the tile menu once; after that the choice is kept.
- **Settings in the tile style**: a setting opened from a tile (Brightness, Theme, Band Plan, Logger and the rest) is shown full screen as a list of values or a number with a bar, and a click returns to its category page.
- **Radio controls go back to the normal screen**: after choosing a Band, Mode, Bandwidth, Step, Volume and the other listening controls, the menu closes so the knob tunes straight away. Settings still return to their page.
- **System → Knob**: 20 clicks or 30 clicks per turn, for the two kinds of encoder fitted to these radios. If the knob skips or needs two clicks per step, try the other setting (it is the same as Half-step Encoder on the web page).
- About → Authors: the RESONIK edition line now reads EP2MHB.


## 2.45-r1 (2026-11-06)

First release of the RESONIK edition, based on ats-mini 2.45. Install by USB (see the README); make a backup first.

### Added

- RESONIK boot screen (symbol, name, version) and RESONIK header on the About screens.
- **CW mode** (Mode → CW): the display shows the CW signal's own frequency, received on the upper sideband with the tone at the centre of the filter (0.5k filter: 500 Hz tone; 1.0k: 1000 Hz; wider: 700 Hz). Defaults to the 0.5 kHz filter and 10 Hz steps.
- **IARU Region 1 band plan** on the tuning scale for 160–10 m (and 60 m): coloured strip for CW, narrow/digital, beacons and all-mode/SSB segments, with the current segment named at the pointer. Settings → **Band Plan**: Off, Scale, or Scale+SSB, which also selects the usual sideband (LSB below 10 MHz, USB above) when you tune into a phone segment.
- **Battery time remaining**: the voltage alternates with an estimate such as `~8h20`. The `~` means a typical consumption is assumed; after about 10 minutes on battery the radio measures its own.
- **Automatic attenuation** for AM (Settings → **Auto Att.**, on by default): when a very strong signal would overload the receiver, attenuation is added in steps, and removed again on weaker stations. Shown as `Att: A12` in the info box. Only while AGC/ATTN is set to On.
- **Tile menu** (Settings → **Menu Style** → Tiles): the menu and settings as a full-screen grid of tiles showing current values (mode, band, volume, step, bandwidth…). The classic menu stays the default.
- Named time zones with automatic summer time (Europe, North America, Australia, New Zealand) alongside the plain UTC offsets. Settings → **Time Zone** (was "UTC Offset"); your previous offset is carried over.
- Dual clock: when the time zone is not UTC, the info box alternates local date and time with UTC time.
- "Time: --:--" in the info box while the clock is not set.
- Web settings: "Use Browser Date/Time" now also picks the browser's named time zone, summer time included.
- Clock sync works on networks that block or delay NTP: three time servers (one by IP address), then the time from a web server (up to three tries). When sync fails, a second line tells why (DNS server and result, web results).

### Changed

- The default time zone is UTC (upstream: UTC−5).
- Settings → Date/Time follows the running clock until you change a value; clicking without a change no longer sets the clock back to the moment the screen was opened.
- Firmware updates over Wi-Fi now come from the RESONIK repository. Release tags look like `v2.45-r1` (upstream version + RESONIK release); the version is shown as `v2.45-R1` on the About screen.
- About → Authors credits the RESONIK edition alongside the original authors.
- Release files are named `resonik-ats-mini-<version>-<variant>.zip` and `resonik-ats-mini-<version>-<variant>-ota.bin`.


## 2.45 (2026-10-22)


### Added

- Added RDS OFF option to turn off RDS. [#388](https://github.com/esp32-si4732/ats-mini/issues/388)


### Fixed

- Fixed missing, truncated, or mixed RDS text and unnecessary station name resets, including corruption from empty RDS responses.
- Improved RDS processing to reduce delayed updates and lost data.
- RDS station names now update only after all four segments have arrived in order, starting with segment zero.

## 2.44 (2026-10-18)


### Added

- Add display, radio, power, and connectivity settings to the Config web page, including Bluetooth Unpair All. Move Audio Clamp configuration from the receiver menu to the web page.


### Changed

- Move network settings and web login credentials to a separate Wi-Fi page, and add Wi-Fi mode selection. Organize Config settings into sections.
- Short pressing the Off Timer menu entry now shuts down the receiver into deep sleep. Clicking still opens the timer settings, where a short press disables the timer.


### Fixed

- Suppress clicks when muting or changing bands or modes on receivers without an audio clamp circuit. Disable Audio Clamp on the Config web page for these receivers.

## 2.43 (2026-10-14)


### Added

- Added VFO A/B tuning slots that persist across restarts. Click the VFO A/B menu item to switch slots, or short press to copy the active slot to the other.
- Added an Off Timer to shut down the receiver after 5–240 minutes. Cycle the power switch to restart the receiver.
- Download a BMP screenshot from the Wi-Fi web interface’s status page, one download at a time.


### Changed

- Keep tuning and menus responsive while connecting to Wi-Fi or synchronizing the clock. Failed time synchronization no longer clears the clock, and changing RDS settings preserves time set via Wi-Fi.
- Remote control over USB, TCP, and Bluetooth is more reliable and no longer freezes receiver controls while waiting for commands.
- Renamed Sleep to Sleep Timer, added a “sec” label, and show “Off” when disabled.


### Fixed

- Fixed saving preferences after button actions, including saving and deleting memory slots. Menu navigation no longer triggers unnecessary saves.
- Preserved SSB fine tuning across band changes and restarts, and fixed recalling memories that differ only below 1 kHz. SSB tuning now uses a fixed 28 kHz tuning grid so the BFO no longer depends on tuning direction. WARNING: this update resets saved band frequencies, modes, steps, bandwidths, and SSB calibration.

## 2.42 (2026-09-30)


### Added

- Add a web-based memory editor with slot names, reordering, and JSON import/export.
- Show memory slot names in the memory menu and below matching frequencies, with RDS station names taking priority.
- Show used and free internal heap and PSRAM in a small, muted footer on each receiver web page.
- The remote `#` command accepts an optional name, and `$` includes saved names in its output.


### Changed

- Reordered the settings menu to group related controls.
- The default layout's RSSI meter now shows empty bars across its full range.

## 2.41 (2026-09-29)


### Added

- An FM Stereo setting to force mono audio instead of letting the receiver blend down to mono by itself, which trades the stereo image for less hiss on weak stations. [#371](https://github.com/esp32-si4732/ats-mini/issues/371)
- Binary screenshot command `c` sends a BMP file directly using half as much data. [#373](https://github.com/esp32-si4732/ats-mini/issues/373)
- Add temporary AVC Hold in LSB/USB via short press in the AVC menu. Rotation resumes automatic AVC; band/mode reinitialization or restart clears Hold.
- Experimental AM/SSB patch sets can be uploaded over Wi-Fi and selected from the Patches page or Settings → DSP Patches. See [discussion #377](https://github.com/esp32-si4732/ats-mini/discussions/377).


### Changed

- FM Region and FM Stereo can be changed in any mode and take effect when using FM. [#371](https://github.com/esp32-si4732/ats-mini/issues/371)
- Faster screenshot capture with more reliable USB transfers. [#373](https://github.com/esp32-si4732/ats-mini/issues/373)
- Scan for Bluetooth HID remotes with a 25% duty cycle instead of 100%, so the Bluetooth radio is busy for a quarter of each scan. [#374](https://github.com/esp32-si4732/ats-mini/issues/374)


### Fixed

- Keep controls responsive while showing Bluetooth, Wi-Fi, and firmware update status messages. [#374](https://github.com/esp32-si4732/ats-mini/issues/374)
- Apply volume changes made while muted and preserve hardware mute across band/mode changes, including on T-Embed.
- Initialize SSB consistently when switching from FM and clear leftover sideband bits when returning to AM or FM, so receiver configuration no longer depends on previously used modes.

## 2.40 (2026-09-19)


### Added

- Check for and install firmware updates over Wi-Fi from Settings → Update FW or the web interface, with a separate expandable form for manual file uploads. Press the encoder to cancel a transfer. [#358](https://github.com/esp32-si4732/ats-mini/issues/358)


### Changed

- The web Memory page now requires the configured login and password.

## 2.39 (2026-09-16)


### Added

- Added an Unpair All Bluetooth menu option that clears all saved BLE bonds and then switches Bluetooth off.
- High contrast Light and Dark color themes ([discussion #366](https://github.com/esp32-si4732/ats-mini/discussions/366)).
- TCP remote control on port 60000, enabled via Settings -> TCP Port -> Ad hoc.


### Changed

- Replaced TFT_eSPI and PNGdec with LovyanGFX for display and splash-screen rendering. The display rendering is now approximately 2.3 times faster.
- Replaced the compile-time encoder half-step option with a persistent runtime setting on the Wi-Fi configuration page.

## 2.38 (2026-09-01)


### Added

- Added date synchronization via NTP and RDS Clock Time; the current date is shown alongside the clock when available.
- Added offline UTC date/time setup from the Settings menu.
- Added offline date/time setup from the web configuration page, and display of the date/time and configured UTC offset on the status page.

## 2.37 (2026-08-28)


### Added

- Added an optional, user-uploadable PNG splash screen that is displayed when the receiver starts.


### Fixed

- Protected web configuration changes and splash image uploads with the configured web interface credentials.

## 2.36 (2026-08-26)


### Added

- Add optional compile-time options to define TX power levels (BLE_POWER_LEVEL, WIFI_POWER_LEVEL), reduce the default WiFi power level from 19.S to 17dBm. See <https://esp32-si4732.github.io/ats-mini/development.html#compile-time-options> for additional details. [#307](https://github.com/esp32-si4732/ats-mini/issues/307)


### Changed

- Make possible using either RSSI or SNR for Squelch. The setting is saved separately for each mode (FM, LSB, USB, AM).
- Try to return the power consumption back to normal when BLE is switched off <https://github.com/esp32-si4732/ats-mini/discussions/324#discussioncomment-17123023>.

## 2.35 (2026-05-03)


### Fixed

- Try to connect to other WiFi access points besides the strongest one (including hidden SSIDs if the corresponding option is enabled). [#326](https://github.com/esp32-si4732/ats-mini/issues/326)
- Do not abort the EiBi download process immediately <https://github.com/esp32-si4732/ats-mini/discussions/325>.

## 2.34 (2026-05-01)


### Added

- "PSRAM not detected" fatal error screen (shown if a wrong f/w variant was flashed).
- Add `E` serial command that emulates encoder short press.
- Add `F` serial command to tune directly to a frequency in Hz within the current band. In SSB modes, sub-kHz digits set the BFO as well.
- Experimental Bluetooth LE control is available under Settings->Bluetooth: use Ad hoc for the BLE remote-control protocol, or HID to connect supported Bluetooth remotes/keyboards for tuning and menu actions. It may be unstable.
- Experimental support for LILYGO T-Embed SI4732, see <https://esp32-si4732.github.io/ats-mini/hardware.html#lilygo-t-embed-si4732> for more details.


### Changed

- Abort the long running operations (Seek, Scan, EiBi) via any remote command
- Change the set theme serial command from ! to ^.
- Drop workaround for https://github.com/espressif/arduino-esp32/issues/11742. Use faster method (WiFiMulti) to connect to 2-nd or 3-rd configured access point.
- Remote control over USB serial port is disabled by default. To enable it, go to Settings->USB Port and set it to Ad hoc mode.


### Fixed

- Add an exhaustive (hopefully) list of UTC offsets. WARNING: please adjust your UTC offset again (the stored index is no longer valid). [#287](https://github.com/esp32-si4732/ats-mini/issues/287)
- Fix out of range menu controls due to fast encoder rotation.

## 2.33 (2025-09-22)


### Changed

- Adjust gamma for display ID 0x04858552 so the themes look closer to how they were designed (at least Orange now doesn't look like a lemon).

## 2.32 (2025-09-16)


### Removed

- Remove the dynamic CPU frequecy feature introduced in v2.31 (it caused sound artifacts when rotating the encoder). [#244](https://github.com/esp32-si4732/ats-mini/issues/244)
- Do not show the "Add" hint on an empty memory slot to prevent confusion with click vs short press.


### Changed

- Avoid drawing background color when drawing text. This dramatically helps UI customization modding efforts (like setting a background image instead of a plain color, [for example](https://github.com/esp32-si4732/ats-mini/discussions/240)). [#239](https://github.com/esp32-si4732/ats-mini/issues/239)
- Move the Web UI credentials form fields below the Wi-Fi settings. [#241](https://github.com/esp32-si4732/ats-mini/issues/241)


### Fixed

- Fix Wi-Fi connection issue to 2nd or 3rd access point configured on the settings web page. [#244](https://github.com/esp32-si4732/ats-mini/issues/244)

## 2.31 (2025-09-13)


### Removed

- Remove faster tuning in Seek mode on SSB and in Scan mode via press & rotate in favor of the new accelerated encoder control.
- Remove the ENABLE_HOLDOFF compile-time option.


### Added

- Encoder acceleration.
- Encoder click now cancels the EiBi schedule download process.


### Changed

- Reduce the upper CB band limit to 28MHz. [#205](https://github.com/esp32-si4732/ats-mini/issues/205)
- Independent USB/LSB calibration values. WARNING: this change will reset the bands settings. [#220](https://github.com/esp32-si4732/ats-mini/issues/220)
- Render partial frequency numbers on the tuning scale around screen edges. [#235](https://github.com/esp32-si4732/ats-mini/issues/235)
- Disable the Memory menu timeout (it is a surfing mode like Seek or Scan). Short press (0.5 sec) saves/clears a slot, click closes the menu.
- EXPERIMENTAL: overclock the I2C bus to 800kHz (affects Si4732).
- Set CPU freq to 240 MHz on encoder rotation, drop back to 80 MHz after 10 seconds of no activity. This results in snappier UI.


### Fixed

- Fix AVC wrapping to avoid selecting odd AVC values. [#207](https://github.com/esp32-si4732/ats-mini/issues/207)
- Add 100ms delay after Si4732 POWER_ON to fix the "Si4732 not detected" issue [#213](https://github.com/esp32-si4732/ats-mini/issues/213)
- Fix misbehaving squelch when changing bands.

## 2.30 (2025-08-07)


### Added

- Add Scan mode. Press the encoder for 0.5 seconds to rescan, press & rotate to tune using a larger step. The scan process can be aborted by clicking or rotating the encoder.


### Changed

- Switch from EEPROM to Preferences library to store the receiver settings. This change removes some old limitations and enables more flexible settings management. WARNING: upgrading to this firmware version from an older one will reset the settings. Also a forced reset might be required (hold the encoder and power on the receiver). [#94](https://github.com/esp32-si4732/ats-mini/issues/94)
- Mute audio amp during seek action to prevent audible artifacts. [#190](https://github.com/esp32-si4732/ats-mini/issues/190)
- Display "Loading SSB" message in the zoomed menu area.
- Extend the 16m broadcast band a bit to include CRI on 17490.
- Increase the number of memory slots to 99.


### Fixed

- Do not lose SSB sub kHz digits when storing Memory slots. [#109](https://github.com/esp32-si4732/ats-mini/issues/109)
- Restore saved bandwidth.
- Use default step when switching modes or memories.

## 2.28 (2025-07-01)


### Added

- Add UTC+5:30 offset for India (Asia/Kolkata). Users with offsets greater than 5:30 might need to readjust their timezone settings (menu indexes have been shifted).
- Enable PSRAM using different build artifacts for OSPI and QSPI ESP32-S3 modules. For more info see <https://esp32-si4732.github.io/ats-mini/flash.html#firmware-files>.


### Changed

- Much better seek sensitivity (SI4735 library patch by @zhang-chong). [#129](https://github.com/esp32-si4732/ats-mini/issues/129)
- 200kHz FM step now uses odd frequencies (99.1, 99.3, etc). [#161](https://github.com/esp32-si4732/ats-mini/issues/161)

### Fixed

- Fix loud clicks when changing bands/modes on the PCB version without the mute circuit. [#103](https://github.com/esp32-si4732/ats-mini/issues/103)
- Do not shadow station names by zoomed menu in the seek mode. [#157](https://github.com/esp32-si4732/ats-mini/issues/157)

## 2.27 (2025-06-07)


### Added

- Allow connecting to the receiver's web UI using the atsmini.local mDNS name in addition to an IP address. [#145](https://github.com/esp32-si4732/ats-mini/issues/145)


### Changed

- Disable Seek mode (menu) timeout.


### Fixed

- Clear RSSI, SNR, and station name when doing normal Seek. [#146](https://github.com/esp32-si4732/ats-mini/issues/146)
- Fix backwards EiBi seek from 30000kHz.

## 2.26 (2025-06-02)


### Added

- Show DHCP-assigned IP address on the About system screen.


### Fixed

- Fix crash when trying download the EiBi schedule in offline mode. [#132](https://github.com/esp32-si4732/ats-mini/issues/132)
- Fix timeout when connecting to Wi-Fi access points.

## 2.25 (2025-05-31)


### Removed

- Disable EEPROM backup/restore option on the settings web page. If you used this feature to restore the EEPROM and now see strange bugs when switching bands, please reset the receiver settings.


### Fixed

- Fix blinking RDS and static frequency name.

## 2.24 (2025-05-30)


### Added

- EiBi schedule support, see https://esp32-si4732.github.io/ats-mini/manual.html#schedule


### Fixed

- Reapply Squelch after waking up from CPU Sleep mode. [#127](https://github.com/esp32-si4732/ats-mini/issues/127)

## 2.23 (2025-05-26)


### Added

- Ability to select FM de-emphasis setting based on region. [#85](https://github.com/esp32-si4732/ats-mini/issues/85)
- Show MAC-address on the receiver status web page. [#114](https://github.com/esp32-si4732/ats-mini/issues/114)
- Add ALL-CT RDS options for those who prefer precise time over WiFi.
- EEPROM backup/restore via the receiver web interface. Restore only works on compatible firmware versions.
- New optional UI layout with large S-meter and S/N-meter.


### Fixed

- Escape quotes in web form fields (like SSIDs & passwords). [#113](https://github.com/esp32-si4732/ats-mini/issues/113)
- Fix the AVC bug, huge thanks to Dave (G8PTN)! [#117](https://github.com/esp32-si4732/ats-mini/issues/117)

## 2.22 (2025-05-23)


### Removed

- Removed the Mute menu option. Use short press instead while in the volume adjustment mode.


### Added

- Experimental Squelch option based on RSSI threshold. Unlikely to work in SSB mode. To turn it off quickly, short press (>0.5 sec) the encoder button in the Squelch menu mode. [#32](https://github.com/esp32-si4732/ats-mini/issues/32)
- Help screen and system info screen (see `Settings->About`). The help screen is also displayed on first start.


### Changed

- Use short press to delete a memory slot.


### Fixed

- Fix restoring memory slots that belong to the bands with the same names. [#100](https://github.com/esp32-si4732/ats-mini/issues/100)

## 2.21 (2025-05-21)


### Changed

- Make the Wi-Fi icon a bit more lightweight.


### Fixed

- Disable the automatic tuning capacitor. [#97](https://github.com/esp32-si4732/ats-mini/issues/97)
- NTP time synchronization no longer ignores seconds.

## 2.20 (2025-05-18)


### Added

- Direct frequency input mode. Press and rotate the encoder to select the step (digit or "half-digit"), rotate the encoder to adjust the frequency, use short press to align frequency to the current step. To exit the mode, click the encoder or wait for a couple of seconds. [#26](https://github.com/esp32-si4732/ats-mini/issues/26)
- `Settings->UTC Offset` now affects the displayed time (whether it was received from RDS or NTP). [#44](https://github.com/esp32-si4732/ats-mini/issues/44)
- `Settings->Scroll Dir.` option to reverse the menu scroll direction. [#79](https://github.com/esp32-si4732/ats-mini/issues/79)
- Add SNR to the serial console log.
- Stop the automatic seek process by clicking the encoder button (exit by rotating it still works).
- Use press+rotate for manual fine tuning in Seek mode.
- Wi-Fi mode to sync time over NTP, view the receiver status and Memory slots.


### Changed

- Experimental: re-enable automatic antenna capacitor on FM, MW and 160M bands (as it was in 1.06 and earlier), plus reset it when switching between AM bands. [#13](https://github.com/esp32-si4732/ats-mini/issues/13)
- The short press (volume) shortcut is no longer global (now works only in VFO mode) and can be used by menus and other modal modes. [#26](https://github.com/esp32-si4732/ats-mini/issues/26)
- SSB tuning is now aligned to the current step. [#76](https://github.com/esp32-si4732/ats-mini/issues/76)
- Completely silence speaker output on mute. [#78](https://github.com/esp32-si4732/ats-mini/issues/78)
- Decrease RSSI & SNR thresholds in FM/AM seek modes.
- Theme editor can be enabled or disabled via the `T` terminal command, without recompiling the firmware.


### Fixed

- Set seek step according to the current step. [#5](https://github.com/esp32-si4732/ats-mini/issues/5)
- SSB memory slots now store frequencies with 100Hz precision. Serial commands that work with memory slots now expect frequencies in Hz. [#79](https://github.com/esp32-si4732/ats-mini/issues/79)
- Fixed wrong menu titles. [#80](https://github.com/esp32-si4732/ats-mini/issues/80)
- Fix audible clicks when scrolling over empty memory slots.


### Improved Documentation

- Add web-based Memory slot edit/backup/restore tool: <https://esp32-si4732.github.io/ats-mini/memory.html>

## 2.14 (2025-05-06)


### Added

- Zoomed menu mode for accessibility purposes (enabled via Settings->Zoom menu). [#71](https://github.com/esp32-si4732/ats-mini/issues/71)
- Serial commands `$` and `#` to backup and restore memory slots.


### Changed

- Extended 75M and 90M band boundaries.

## 2.13 (2025-05-01)


### Added

- Add Seek menu - automatic scan on AM/FM, faster tuning step on SSB. [#26](https://github.com/esp32-si4732/ats-mini/issues/26)
- CPU sleep mode (light sleep) to reduce the power consumption even more.


### Changed

- Increase seek timeout from 8 seconds to 10 minutes.


### Fixed

- Get rid of the short display blink that shows some visual garbage at power on.

## 2.12 (2025-04-29)


### Removed

- Remove the main menu counter.


### Added

- Added RDS radio text, program type, and PI code. Can be enabled via Settings->RDS. [#9](https://github.com/esp32-si4732/ats-mini/issues/9)
- Memory menu to store favorite stations. Press the encoder on an empty slot to store the current frequency and mode, rotate the encoder to select a slot. WARNING: this firmware version resets the receiver settings. [#56](https://github.com/esp32-si4732/ats-mini/issues/56)
- Add `I` and `i` hotkeys to tweak the calibration value. Add separate BFO and calibration fields to the remote log. [#60](https://github.com/esp32-si4732/ats-mini/issues/60)
- Add 10kHz step for WFM mode
- Add `Settings->Sleep Mode`. Controls whether the encoder is locked or not during sleep. Unlocked mode allows tuning (and setting the volume) with less self-induced noise.


### Changed

- Remote encoder keys changed. Use `R` and `r` to rotate it and `e` to click.


### Fixed

- Get rid of 1M and other AM steps in LSB/USB steps menu [#15](https://github.com/esp32-si4732/ats-mini/issues/15)
- Extend the 25, 31, 41, 49, and 60 meter bands. [#50](https://github.com/esp32-si4732/ats-mini/issues/50)

## 2.11 (2025-04-22)


### Added

- Added named frequencies database with FT8 and SSTV frequencies as samples.
- New Magenta color theme


### Changed

- Make RDS clock optional, move it to the info panel. To enable it, set `Settings->RDS` to `PS+CT`. Also clock is synchronized only once until the receiver or RDS CT is turned on and off (this helps to avoid getting wrong time from incorrectly configured FM stations).  WARNING: this firmware resets the receiver settings! [#39](https://github.com/esp32-si4732/ats-mini/issues/39)
- Added DID/DST registers display to About screen.
- Added RGB color bar to About screen.
- Set the default sleep timeout to 0 (disabled)


### Fixed

- Use BGR color order on the GC9307 display type [#41](https://github.com/esp32-si4732/ats-mini/issues/41)
- Fixed display of CB channels. [#51](https://github.com/esp32-si4732/ats-mini/issues/51)
- SSB calibration wasn't applied when changing bands or modes. Now it is. [#53](https://github.com/esp32-si4732/ats-mini/issues/53)

## 2.10 (2025-04-18)


### Changed

- Move Calibration to the Settings menu
- New bands list: more bands, different names and limits, sorted by modulation type for faster switching. WARNING: this change resets the receiver settings!
- Pixel-wise tuning scale scrolling
- Simplify remote serial output, change encoder keybindings. Use `E` and `e` to simulate the encoder rotation, `p` to push the button.


### Fixed

- Fix the screenshot palette. You can use the following oneliner to make a screenshot: `echo -n C | socat stdio /dev/cu.usbmodem14401,echo=0,raw | xxd -r -p > /tmp/screenshot.bmp` [#40](https://github.com/esp32-si4732/ats-mini/issues/40)
- Autodetect the display type and invert/mirror the picture if needed [#41](https://github.com/esp32-si4732/ats-mini/issues/41)
- Fix SSB band limits checks [#46](https://github.com/esp32-si4732/ats-mini/issues/46)
- Hopefully fix the SSB noise tone issue [#46](https://github.com/esp32-si4732/ats-mini/issues/46)
- The "EEPROM Resetting" screen is not hidden

## 2.00 (2025-04-15)


### Added

- New Space theme.


### Changed

- Major code refactoring: split code into modules, get rid of global variables, etc. Huge thanks to Marat Fayzullin for doing the heavy lifting. [#35](https://github.com/esp32-si4732/ats-mini/issues/35)
- Bump ESP32 Arduino core to 3.2.0
- Disable delayed screen update by default
- Remove the Z timezone marker from RDS clock.

## 1.09 (2025-04-03)


### Added

- + Now aligning frequency to the step when tuning in AM and FM modes. [#30](https://github.com/esp32-si4732/ats-mini/issues/30)
- **RDS Time Synchronization**: Added support for displaying the current time synchronized via RDS (Radio Data System) when tuned to FM stations broadcasting time information. The time is displayed below the battery icon and includes a "Z" suffix to indicate UTC time. The time display is only visible after successful synchronization with an RDS signal. [#34](https://github.com/esp32-si4732/ats-mini/issues/34)


### Changed

- Make the Night color theme less bright, fix theme length

## 1.08 (2025-03-25)


### Added

- Support press+rotate for faster SSB tuning [#4](https://github.com/esp32-si4732/ats-mini/issues/4)
- Add channel information for CB. Support European and Russian channel lists from A to H. [#21](https://github.com/esp32-si4732/ats-mini/issues/21)


### Changed

- The default CB channel has been changed to the most popular frequency in Russia 27135 kHz (C15E). [#21](https://github.com/esp32-si4732/ats-mini/issues/21)
- Remove some delays and extra screen repaints (might or might not make the tuning process a bit faster)


### Fixed

- Fix getLastStep for LW/MW [#18](https://github.com/esp32-si4732/ats-mini/issues/18)
- Fix the volume shortcut bug with active menu [#20](https://github.com/esp32-si4732/ats-mini/issues/20)

## 1.07 (2025-03-21)


### Fixed

- Reenable USB Serial [#12](https://github.com/esp32-si4732/ats-mini/issues/12)
- Fix ANTCAP value readings [#13](https://github.com/esp32-si4732/ats-mini/issues/13)
- Disable automatic antenna capacitor [#13](https://github.com/esp32-si4732/ats-mini/issues/13)

## 1.06 (2025-03-20)


### Changed

- Unmute the audio on volume middle press shortcut [#1](https://github.com/esp32-si4732/ats-mini/issues/1)
- Lock encoder rotation during sleep, [#8](https://github.com/esp32-si4732/ats-mini/issues/8)
- Redraw the battery indicator in iOS style [#11](https://github.com/esp32-si4732/ats-mini/issues/11)
- Print tuning capacitor value to Serial port

## 1.05 (2025-03-16)


### Added

- Add EEPROM write icon
- New color themes: Bluesky, eInk, Pager, Orange, Night, Phosphor
- Support for color themes
- Theme editor. To enable it, recompile the firmware with THEME_EDITOR=1 and connect via USB serial port. Press @ to print the current theme, change it using any text editor (see themes.h for details), then press ! and paste the updated theme (effective until the receiver is powered off). Once you are happy, add the resulting colors to themes.h. Check out a useful color picker as well https://chrishewett.com/blog/true-rgb565-colour-picker/


### Changed

- Add repo URL to the About screen, hide battery


### Fixed

- Fix tuner scale height on MW1 & MW2 bands

## 1.04 (2025-03-12)


### Changed

- Refactor the code to make it more DRY
- Return the Volos Project UI with some changes. The new s-meter also serves as a stereo indicator on FM.

## 1.03 (2025-03-11)


### Added

- Add screenshot feature, new command to toggle serial log. Send "C" over the serial port to make a screenshot, then feed the resulting HEX dump to the "xxd -r -p" command to get a BMP image. Use the "t" command to toggle the serial log.


### Changed

- Center and underline the menu headers. Thanks to R8ADR for the patch


### Fixed

- Fix "P" (button press) serial command

## 1.02 (2025-03-11)


### Added

- * Display timeout has been added (Settings > Sleep), a value of zero disables the timeout
  * Turning the display off/on by long pressing the encoder (2 seconds). This reduces QRM from the display
  * Added "O" and "o" commands (display off and on) to the serial port protocol
- AM/FM station search by scrolling with the encoder pressed (the search can also be stopped by rotating the encoder)
- Pressing the encoder for more than 0.5 seconds triggers volume adjustment (this compensates the Volume menu option no longer being highlighted by default)


### Changed

- * The menu has been reorganized, and the Spare items have been removed
  * A Settings menu has been added, and the display brightness adjustment option has been moved there
  * The position in the main menu and settings menu is remembered (until power is turned off)
  * The startup splash screen has been moved to Settings > About (but the reset of settings by turning on the receiver with the encoder pressed still works)
  * The Seek Up/Down menu items have been removed
- Changed the charging indicator (lightning icon instead of the EXT text)


### Fixed

- Fixed the duplication of the MW2 band name

## 1.01 (2025-03-11)

Identical to 1.01 by Dave (G8PTN), just recompiled for `esp32:esp32@3.1.3` Arduino core.

### Added

- Added "MODE" configuration per band (FM, AM, LSB, USB)


### Changed

- Improved tuning speed by delaying the display updates

## 1.00 (2025-03-11)

Identical to 1.00 by Dave (G8PTN), just recompiled for `esp32:esp32@3.1.3` Arduino core.

### Added

- Added "Brightness" menu option

  * This controls the PWM from 32 to 255 (full on) in steps of steps of 32
  * When the brightness is set lower than 255, PSU or RFI noise may be present
- Added "Calibration" menu option

  This allows the SI4732 reference clock offset to be compensated per band
- Added Automatic Volume Control (AVC) menu option. This allows the maximum audio gain to be adjusted.
- Added GPIO1 (Output) control (0=FM, 1 = AM/SSB)
- Added a REMOTE serial interface for debug control and monitoring
- User interface modified:

  * Removed the frequency scale
  * Set "Volume" as the default adjustment parameter
  * Modifed the S-Meter size and added labels
  * All actions now use a single press of the rotary encoder button, with a 10s timeout
  * Added status bar with indicators for Display and EEPROM write activity
  * Added unit labels for "Step" and "BW"
  * Added SSB tuning step options 10Hz, 25Hz, 50Hz, 0.1k and 0.5k
  * Added background refresh of main screen
- VFO/BFO tuning mechanism added based on Goshante ATS_EX firmware

  * This provides "chuff" free tuning over a 28kHz span (+/- 14kHz)
  * Compile option "BFO_MENU_EN" for debug purposes, manual BFO is not required


### Changed

- Modified FM steps options (50k, 100k, 200k, 1M)
- Modified the audio mute behaviour

  * Previously the rx.setAudioMute() appeared to unmute when changing band
  * The "Mute" option now toggles the volume level between 0 and previous value
- Modified the battery monitoring function

  * Uses set voltages for 25%, 50% and 75% with a configurable hysteresis voltage
  * Added voltage reading to status bar
- Settings for AGC/ATTN, SoftMute and AVC stored in EEPROM per mode

  AGC/ATTN (FM, AM, SSB), SoftMute (AM, SSB), AVC (AM, SSB)


### Fixed

- Fix compilation errors related to ledc* calls

  https://docs.espressif.com/projects/arduino-esp32/en/latest/migration_guides/2.x_to_3.0.html#ledc
