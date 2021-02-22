# Real-time Crypto Currency Monitor

This monitor is a command line dashboard, it uses [ncurses](https://www.gnu.org/software/ncurses/), in combination with the [Binance API ](https://github.com/binance/binance-spot-api-docs/blob/master/rest-api.md) where it fetches all the data from. 

Crypto currency data is readily available from `Binance API` end points, not only that, but we are able to make a ludicrous amount of requests (up to 1200 request weight per minute!). We can take profit of that and develop cool tools, such as this present command line dashboard.

Right now it can also be linked to your personal `Binance` portfolio using your `api key` and `api secret key`. It will only display your current portfolio cryptos, as well as an overview of the total value in `USDT`



<img src="https://github.com/edghyhdz/crypto_monitor/blob/main/images/timelapse.gif"  alt="Crypto Monitor"/>

Conntact me on twitter [![Tweet](https://img.shields.io/twitter/url/http/shields.io.svg?style=social)](https://twitter.com/messages/compose?recipient_id=46040819) | [Create an issue](https://github.com/edghyhdz/crypto_monitor/issues/new)

---

## Table of Contents

1. [Disclaimer](#disclaimer)
2. [Description](#description)
3. [Usage](#usage)
4. [Dependencies](#dependencies)
5. [Installation](#installation)
6. [References](#references)
7. [Author](#author)

## Disclaimer
I know cryptocurrency might be a hot polarizing topic. This project was done to exploit the fine resolution data that is easily accessible for everyone. Crypto trading should not be seen as a game at all. 

I am by no means a crypto currency expert. This project was done out of curiosity and the fact that there is a lot of data to fetch out there, which made a project like this interesting to do. 

The usage you might give to the application is at your own risk. Crypto currency trading is not a game and should not be seen as one. 

## Description

This command line dashboard was built using [ncurses](https://www.gnu.org/software/ncurses/), in combination with [Binance API](https://github.com/binance/binance-spot-api-docs/blob/master/rest-api.md). 

Crypto currency data is fetched using the following `Binance API` end points:
  1. https://api.binance.com/api/v3/aggTrades
  2. https://api.binance.com/api/v3/ticker/price
  3. https://api.binance.com/api/v3/ticker/account (HMAC SHA256)<sup>1</sup>
  
<sup>1</sup> Requires HMAC SHA256 signature

End points 1 and 2 are used to get historical data, `aggTrades` is used within the code to get all aggregrated currencies that should be listed in `Orchestrator.cpp::Orchestrator::currencies` vector, 

```c++
Orchestrator::Orchestrator(bool wallet) : _wallet(wallet){
  std::vector<std::string> currencies{"BTCUSDT", "REEFUSDT"};
  ...
 
}
```
You can add more to the `currencies` vector, these will be queried once every 10 seconds. This task is launched in threads, meaning that the requests to binance will be done in parallel (in case you are querying more than one currency). It is important to notice that you have a `request weight` limit! More info on that on the official [Binance API](https://github.com/binance/binance-spot-api-docs/blob/master/rest-api.md) website

End point 2, is the one used to fetch the data that is used for the dashboard. This one is querying data every two seconds. 

The third endpoint will only work if you set to `true` the `WALLET` variable in `ncurses_display.h`, 

```c++
#define WALLET false

// Change to 
#define WALLET true
```

It is per default in `false`. If you do change it to `true`, please notice that you have to add the following env variables into your `.bashrc` file. 
```sh
BINANCE_API_KEY="YOUR API KEY"
BINANCE_API_SECRE="YOUR API SECRET"
```

These will be used eventually in `QueryCrypto.cpp::QueryCrypto::getRequest()`, 
```c++
  ...
  // Headers and set up signature
  if (wallet){
    const char *API_KEY = std::getenv("BINANCE_API_KEY");
    const char *API_SECRET = std::getenv("BINANCE_API_SECRET");
```

Adding your `api key` information will make the dashboard able to display your current wallet information, as displayed below, 

<img src="https://github.com/edghyhdz/crypto_monitor/blob/main/images/wallet_tracker.gif"  alt="Wallet Tracker"/>

Displaying then, information related to all currencies you hold plus the `total potfolio value` in `usdt`. There is some information that will be always displayed as per default, this being the `current request weight`, that resets to `0` every `1 minute`. 

## Usage
> **Tested in linux only**

**Starting the dashboard**

```sh
source cryptomonitor
```

The dashboard will show in the terminal with some default coins to display. It might take a couple of seconds until it has requested some data from the `Binance api` end point to fully display a crypto currency time series. 

**Selecting different currencies**

Click `F1`, this will open a selection window (modified code from [flarn2006's](https://github.com/flarn2006/MiscPrograms/blob/master/graph.c) repo) 

<img src="https://github.com/edghyhdz/crypto_monitor/blob/main/images/select_window.gif"  alt="Selection Window"/>

You can select up to 3 different `cryptos` to display on the dashboard.

You can also select the amount of data to display. The deault is `180` data points. That corresponds to 360 seconds of data, or a `6 minute` window frame.


## Dependencies

 1. [ncurses](https://www.gnu.org/software/ncurses/)```sh sudo apt install libncurses5-dev libncursesw5-dev```
 2. [libcurl](https://curl.se/libcurl/) ```sh sudo apt-get install -y libcurl-dev```
 3. [Open SSL](https://www.openssl.org/) ```sh sudo apt-get install libssl-dev```
 4. [cmake](https://www.gnu.org/software/make/) ```sh sudo apt-get -y install cmake```

## Installation

Clone this repository like so, 
 ```sh
 git clone https://github.com/edghyhdz/crypto_monitor.git
 ```
Once inside the root project folder `crypto_monitor`,
 ```sh
 mkdir build && cd build
 cmake ..
 source install.sh
 ```
`install.sh` will run the final installation that will create a terimal shortcut named `cryptomonitor`. 

The final project folder structure is the following, 

    .
    ├── ...
    ├── build                     # Directory were project was built
    │   ├── executable            # Executable location
    │       ├── data_test         # csv data used by the dashboard <- all data will be in these two folders
    │       └── data_aggregated   # aggregated csv data -not used by the dashboard-
    └── ...


If everything was done allright, you should be able to run the dashboard by doing the following, 
```sh
source cryptomonitor
```

## References

There was one incredible repository I happen to come accross, this was [flarn2006](https://github.com/flarn2006)'s repository [MiscPrograms](https://github.com/flarn2006/MiscPrograms/blob/master/graph.c). 

This repo included an amazing script to do plots using the `ncurses` library. I took parts of his/her script, particularyl this one [graph.c](https://github.com/flarn2006/MiscPrograms/blob/master/graph.c), and adapted it to plot in a real-time basis. 

All other references can be found inside the code.

## Author

Edgar Hernandez 
