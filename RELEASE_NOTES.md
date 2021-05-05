# HodlCash Release Notes for v1.2.0.0

Changes in 1.2.0.0

- NEW - updated splash screen with new tagline
- NEW - links navigation menu for website, blockexplorer 1, blockexplorer 2, paper wallet, github, white paper and road map
- NEW - social navigation menu for Facebook, Facebook Group, Twitter, Google Plus, Youtube, Reddit, Telegram Group and Telegram Announcements

This release adds new functionality to the wallet to aid users in quickly finding all our important internet links and social media platforms. v1.2.0.0 is not a mandatory release and is backwards compatible with v1.1.7.0

# HodlCash Release Notes for v1.1.7.0

Changes in 1.1.7.0

- CRITICAL - Emergency fix for windows wallets stuck on block 143707
- FIX - issue #58 reported by Cryptohub exchange and fixed by FreeTrade
- FIX - encrypted wallet are now able to sign messages
- FIX - seeds list was adjusted
- NEW - additional logging was added to troubleshoot issue #65 

This release mainly impacts Windows users and so all Windows users are advised to upgrade to v1.1.7 because a bug prevented windows wallets from being able to synchronize pass block 143707

# HodlCash Release Notes for v1.1.6.0

Changes in 1.1.6.0

## Emergency Release

This release corrects variations in rate calculations to prevent future unexpected forks.

This release is an emergency mandatory upgrade for all users ! it contains a fix that takes immediate effect against the fork type we have seen so far.

This release overwrites all previous release, there will be a hard fork at block 75100 , at which point all older releases <1.1.6 will be rejected and considered as obsolete.

# HodlCash Release Notes for v1.1.5.0

Changes in 1.1.5.0

- FIX - issue #10 added 2 new RPC calls , deposittoaddress and listtermdeposits
- BUG - issue #49 TheTDD PR merge to fix hash rate accuracy in solo miner
- BUG - Optiminer fix to getPostRateInterest() function doing too many math.pow() calls affecting performance on large wallets and slower cpu's.
- BUG - added missing background bitmaps for setup.exe 

# HodlCash Release Notes for v1.1.4.0

Important Notice:

We still see many users using old obsolete wallets, for this reason there will be a hard fork at block 75000.
IF you are on the wrong chain ( not the same height as the block explorer ) , you will need to resync, this will be faster with v1.1.4.

Changes in 1.1.4.0

- NEW - issue #15 a new flag -autoban (default is off) was introduced to automaticaly ban peers for 24hrs if they returned "Connection Refused" or "Host unreachable". 
        This will reduce the cpu load when handling peers. ( recommended for 24/7 servers and VPS )
- NEW - issue #16 a new flag -banobsoleteversion (default is off) was introduced to automaticaly ban peers for 24hrs who have obsolete old wallet version ( < v1.1.4 )
- NEW - issue #40 matured coins will now show an alert bubble ( just like when you find a new block )
- FIX - issue #39 TheTDD has rebuild the term deposit table from scratch and fixed the sorting issues ( should also be faster to sort and update ).
- NEW - issue #47 TheTDD has implemented a deposit table delay mechanism to slow down UI updates during block sync at startup.
        Note: It can take up to 5s before you see your term deposits, but it will speed up block syncing.
- BUG - issue #19 the wallet rpcport was wrongly configured and conflicted with the p2p port, this was the reason for the slow syncing and low peer counts issues in the past. 
        The rpcport is now by default set to 3376
- NEW - issue #22 the sending addressbook will now contain the send and receive addresses combined , this will facilitate sending matured coins or combining inputs to yourself.
- FIX - rpc gettransaction call will now show credit value without interest ( requested by elbandi, to help pool operators )
- FIX - issue #42 estimated date is now based on 120s blocktime , this should provide a more accurate Estimated Date value.
- FIX - issue #44 setban rpc command can now accept a custom bantime ( larger than 24hrs )

The biggest new feature is the faster block loading times. The first time you start the new wallet it will still take as long as before, because 
it is building up a new hash index database ( You will see a new sub folder called "cache" in the blocks folder ). *You must let the loading process complete* 
The second time you restart the new wallet you will see a significantly faster loading time. This new cache only requires about 30 Mb extra disk space.

HodlCash core v1.1.4.0 is now available at:
https://github.com/HodlCash/HodlCash/releases/

Please report bugs using the issue tracker at github, we will not use bitcointalk forum for handling bug reports.
https://github.com/HodlCash/HodlCash/issues

## Upgrading and downgrading
### How to Upgrade

If you are running an older version, shut it down. Wait until it has completely
shut down (which might take a few minutes for older versions).
Backup your wallet.dat file ( which you should always do regardless of an upgrade event )
then copy over HodlCash-qt.exe (on Windows) or just copy over HodlCash-qt (on Linux).

Credits:
- TheTDD
- Elbandi
- ghobson2013

# HodlCash Release Notes for v1.1.3.0

## Optional BETA Release to help System Administrators

In my daily duty to manage the seed nodes and block explorer , I often encounter malicious peer nodes.
To help administrators out there I have added the following 3 new RPC calls, 
These RPC calls are available both via HodlCash-cli and the GUI debug window.

* setban ip or ip/netmask add/remove  - manually add or remove a peer to ban list
* listbanned  -  list banned peers
* clearbanned -  clears all banned

Note: banlist is persisted in banlist.dat file, so the server will remember on restart.

Example:

```code
HodlCash-cli setban 10.10.10.15 add
HodlCash-cli listbanned
 [
   {
       “address” : “10.10.10.15/255.255.255.255",
       “banned_until” : 1514554316,
       “ban_created” : 1514467916,
       “ban_reason” : “manually added”
   }
 ]
 HodlCash-cli getpeerinfo | grep 10.10.10.15
  gone !!
```

Note: This can also be helpful in temporarily banning peers who are on the wrong chain.
Warning: this is a PRE-RELEASE, I am still endurance testing it, its in use on our seed nodes.

# HodlCash Release Notes for v1.1.2.0

## Emergency Release
The previous critical bug fix in v1.1.1 created a consensus disagreement between 2 windows miners mining the same block, Miner1 was on v1.1.0 and Miner2 on v1.1.1, both had a different result when calculating fees. This caused a fork in our block chain at height 29732

To resolve this situation we are hard forking at block nr. 31000 to make sure all miners are on v1.1.2.

## Checking your blockchain
In addition to mandatory upgrade, you may need to reindex your blockchain to the same chain our 
exchanges and block explorer are using.

Go to Help->debug window-> console tab and type "getblockhash 29732"
if you see "0000006aef642158383b16bd16b67d762a41a63ae2cfbdc6479d65d229818175" then you
are on the correct chain, otherwise you need to:
 * delete your blockchain folders named "blocks" and "chainstate" usually stored in c:\users\<USER>\AppData\Roaming\HodlCash and on linux in $HOME/.HodlCash folder.
 * download our bootstrap-27122017-chain-x175.zip file
 * unzip it 
 * run HodlCash-qt.exe -loadblock=c:\FULL-PATH-TO\bootstrap.dat 

## How to Upgrade

If you are running an older version, shut it down. Wait until it has completely shut down (which might take a few minutes for older versions). 
Backup your wallet.dat file ( which you should always do regardless of an upgrade event ) then just copy over HodlCash-qt.exe (win64) or HodlCash-qt (linux).

# HodlCash Release Notes for v1.1.1.0

This release contains one critical and some minor fixes.

- issue #6 A variable initialisation bug in the interest rate calculation caused 
           windows wallet to crash. (found by elbandi)

- issue #7 The windows miner stops after encountering a block validation error 
           due to "too high fee". (caused by issue 6)
           We added a retry mechansim around runtime errors, the timeout is 
           configurable via a new flag called "-minerblocktimeout" (default 30000 ms) 

Other minor fixes:
- max outbound connections was increased from 8 to 16
- hodlcash.icns was fixed for the MAC OSX wallet
- build instructions are now provided for MAC OSX, CentOS7 and Fedora27

Both issues were only affecting windows users, but we still recommend all users to upgrade to v1.1.1

HodlCash core v1.1.1.0 is now available at:
https://github.com/HodlCash/HodlCash/releases/

Please report bugs using the issue tracker at github:
https://github.com/HodlCash/HodlCash/issues

## Upgrading and downgrading
### How to Upgrade

If you are running an older version, shut it down. Wait until it has completely
shut down (which might take a few minutes for older versions).
Backup your wallet.dat file ( which you should always do regardless of an upgrade event )
then run the installer (on Windows) or just copy over HodlCash-qt (on Linux).

Credits:

* PlainKoin
* DisasterFaster
* ghobson2013
* elbandi

Testers Credits:

* mjentsch
* TheTDD 

# HodlCash Release Notes for v1.1.0.0

This release contains one critical and some minor fixes.

- The critical fix is for the term deposit interest rate algorithm bug that kept increasing on every block.
- The new wallet will make sure that if you have pre-fork locked term deposits, the maturation interest will remain unchanged.
- New term deposits created after the hardfork will obey the new term deposit rates:

| Time     | Start Coins | End Coins | Coin Gain | Interest |
| :--------: | :--------: | :--------: | :--------: | :--------: |
| 1 week | 1000 | 1021 | 21 | 2,10% |
| 2 weeks | 1000 | 1043 | 43 | 4,30% |
| 1 month | 1000 | 1096 | 96 | 9,60% |
| 3 months | 1000 | 1331 | 331 | 33,10% |
| 6 months | 1000 | 1929 | 929 | 92,90% |
| 12 months | 1000 | 6933 | 5933 | 593,30% |

The PoS calculation has been adjusted to 15% APR 

**Note:** the old HOdl per block 30 day interest still applies on top of HODL PoS rate.

We also applied cosmetic branding changes to the Wallet splashscreen and background.

## Hardfork at block 25000
### Notice to miners:
* If you are solo-mining you must upgrade to v1.1.0.0 before reaching block 25000 , any blocks found with the old wallet after the fork will be on the wrong chain !.
* If you are pool-mining make sure to check with the pool owner that their wallet has upgraded to v1.1.0.0 before block 25000
* Exchange owners please upgrade to v1.1.0.0 as soon as possible.

HodlCash core v1.1.0.0 is now available at:
https://github.com/HodlCash/HodlCash/releases/

Please report bugs using the issue tracker at github:
https://github.com/HodlCash/HodlCash/issues

## Upgrading and downgrading
### How to Upgrade

If you are running an older version, shut it down. Wait until it has completely
shut down (which might take a few minutes for older versions).
Backup your wallet.dat file ( which you should always do regardless of an upgrade event ) 
then run the installer (on Windows) or just copy over HodlCash-qt (on Linux).

### Downgrade warning

Because this release issues a hard-fork of the network, downgrading to prior versions
will not be possible once the network reaches a block height of 25000.

## Credits:
