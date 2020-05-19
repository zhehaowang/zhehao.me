# Survey on web / app automation / scraping

A reasonable amount of our projects are dedicated to automating particular chores we perform manually, and this documents various methodologies and what has (not) worked in the past.

### Methodology

* Official API
* Protocol
  * http(s) request / response pattern. Consider various platforms (Web / Android / iOS app / Win / MacOS)
  * proxy / self man-in-the-middle
  * Python requests, scrapy
* UI automation
  * Selenium
  * Appium
  * AppleScript
  * Browser plug-in
* Platform-specific hooks


### Case study

* Google API. Official, wide language coverage, reasonably uniform across services and well documented. Example projects
  * [Issue tracker - SpreadSheet r/w](https://github.com/zhehaowang/issue-tracker)
  * [Apartment finder - Maps distance query](https://github.com/zhehaowang/Craigslist-nyc-apartment)
  * [Search for Global Song - Youtube Data API](https://github.com/remap/charles-fos), private
* Craigslist. Query pattern straightforward enough from request / response. [Apartment finder - listing query](https://github.com/zhehaowang/Craigslist-nyc-apartment)
* French embassy. The particular query for vacancy wasn't buried too deep in the JS, although a token is required. The website has since been deprecated.
* US embassy captcha. Selenium-based scraping.
* Instagram. Appium. Caveat being an x86 build needs to be available.
* Du. No desktop / web interface provided. Request / response pattern via Charles. Latest version of Android does not seem to like arbitrary certificates too well. Client-side token mechanism. 
* CoffeeMeetsBagel. No desktop / web interface provided. Request / response pattern via Charles. [Programmatic retrieval of app content](https://github.com/zhehaowang/alighieri)
* Amazon Fresh. Query pattern straightforward enough from request / response. Naive request / response. [AppleScript approach](https://github.com/ahertel/Amazon-Fresh-Whole-Foods-delivery-slot-finder).
* WeChat
  * Back in 2018 WeChat web protocol was available well studied, since that was shut down, current possibilities seem to include
  * iPad / MacOS protocol crack. Charles on the latter suggests an unknown protocol / encoding on port 80 (endpoints vary, and the payload octet stream seems unintelligible). Other parties appear to be [monetizing iPad / MacOS protocols crack](https://github.com/wechaty/wechaty-puppet-padplus). Their approach seems to be selling a token through which a client side can talk with their server which logs onto WeChat and translates client messages to engineered WeChat requests.
  * Windows app hooks. Tooling required and presumably too platform / build specific to scale. [Notes](https://bbs.pediy.com/thread-252058.htm) on finding out a stored QR code png in memory of running wechat process and dll injection.
