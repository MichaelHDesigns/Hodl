Release Process
====================

* update translations (ping wumpus, Diapolo or tcatm on IRC)
* see https://github.com/hodlcash/hodlcash/blob/master/doc/translation_process.md#syncing-with-transifex

* * *

###update (commit) version in sources

	contrib/verifysfbinaries/verify.sh
	doc/README*
	share/setup.nsi
	src/clientversion.h (change CLIENT_VERSION_IS_RELEASE to true)

###tag version in git

	git tag -s v(new version, e.g. 0.8.0)

###write release notes. git shortlog helps a lot, for example:

	git shortlog --no-merges v(current version, e.g. 0.7.2)..v(new version, e.g. 0.8.0)

* * *

###update Gitian

 In order to take advantage of the new caching features in Gitian, be sure to update to a recent version (`e9741525c` or later is recommended)

###perform Gitian builds

 From a directory containing the hodlcash source, gitian-builder and gitian.sigs
  
    export SIGNER=(your Gitian key, ie bluematt, sipa, etc)
	export VERSION=(new version, e.g. 0.8.0)
	pushd ./hodlcash
	git checkout v${VERSION}
	popd
	pushd ./gitian-builder

###fetch and build inputs: (first time, or when dependency versions change)

	mkdir -p inputs
	wget -P inputs https://hodlcashcore.org/cfields/osslsigncode-Backports-to-1.7.1.patch
	wget -P inputs http://downloads.sourceforge.net/project/osslsigncode/osslsigncode/osslsigncode-1.7.1.tar.gz

 Register and download the Apple SDK: (see OS X Readme for details)

 https://developer.apple.com/devcenter/download.action?path=/Developer_Tools/xcode_6.1.1/xcode_6.1.1.dmg

 Using a Mac, create a tarball for the 10.9 SDK and copy it to the inputs directory:

	tar -C /Volumes/Xcode/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/ -czf MacOSX10.9.sdk.tar.gz MacOSX10.9.sdk

###Optional: Seed the Gitian sources cache

  By default, Gitian will fetch source files as needed. For offline builds, they can be fetched ahead of time:

	make -C ../hodlcash/depends download SOURCES_PATH=`pwd`/cache/common

  Only missing files will be fetched, so this is safe to re-run for each build.

###Build HodlCash Core for Linux, Windows, and OS X:

	./bin/gbuild --commit hodlcash=v${VERSION} ../hodlcash/contrib/gitian-descriptors/gitian-linux.yml
	./bin/gsign --signer $SIGNER --release ${VERSION}-linux --destination ../gitian.sigs/ ../hodlcash/contrib/gitian-descriptors/gitian-linux.yml
	mv build/out/hodlcash-*.tar.gz build/out/src/hodlcash-*.tar.gz ../
	./bin/gbuild --commit hodlcash=v${VERSION} ../hodlcash/contrib/gitian-descriptors/gitian-win.yml
	./bin/gsign --signer $SIGNER --release ${VERSION}-win-unsigned --destination ../gitian.sigs/ ../hodlcash/contrib/gitian-descriptors/gitian-win.yml
	mv build/out/hodlcash-*-win-unsigned.tar.gz inputs/hodlcash-win-unsigned.tar.gz
	mv build/out/hodlcash-*.zip build/out/hodlcash-*.exe ../
	./bin/gbuild --commit hodlcash=v${VERSION} ../hodlcash/contrib/gitian-descriptors/gitian-osx.yml
	./bin/gsign --signer $SIGNER --release ${VERSION}-osx-unsigned --destination ../gitian.sigs/ ../hodlcash/contrib/gitian-descriptors/gitian-osx.yml
	mv build/out/hodlcash-*-osx-unsigned.tar.gz inputs/hodlcash-osx-unsigned.tar.gz
	mv build/out/hodlcash-*.tar.gz build/out/hodlcash-*.dmg ../
	popd
  Build output expected:

  1. source tarball (hodlcash-${VERSION}.tar.gz)
  2. linux 32-bit and 64-bit dist tarballs (hodlcash-${VERSION}-linux[32|64].tar.gz)
  3. windows 32-bit and 64-bit unsigned installers and dist zips (hodlcash-${VERSION}-win[32|64]-setup-unsigned.exe, hodlcash-${VERSION}-win[32|64].zip)
  4. OS X unsigned installer and dist tarball (hodlcash-${VERSION}-osx-unsigned.dmg, hodlcash-${VERSION}-osx64.tar.gz)
  5. Gitian signatures (in gitian.sigs/${VERSION}-<linux|{win,osx}-unsigned>/(your Gitian key)/

###Next steps:

Commit your signature to gitian.sigs:

	pushd gitian.sigs
	git add ${VERSION}-linux/${SIGNER}
	git add ${VERSION}-win-unsigned/${SIGNER}
	git add ${VERSION}-osx-unsigned/${SIGNER}
	git commit -a
	git push  # Assuming you can push to the gitian.sigs tree
	popd

  Wait for Windows/OS X detached signatures:
	Once the Windows/OS X builds each have 3 matching signatures, they will be signed with their respective release keys.
	Detached signatures will then be committed to the hodlcash-detached-sigs repository, which can be combined with the unsigned apps to create signed binaries.

  Create the signed OS X binary:

	pushd ./gitian-builder
	./bin/gbuild -i --commit signature=v${VERSION} ../hodlcash/contrib/gitian-descriptors/gitian-osx-signer.yml
	./bin/gsign --signer $SIGNER --release ${VERSION}-osx-signed --destination ../gitian.sigs/ ../hodlcash/contrib/gitian-descriptors/gitian-osx-signer.yml
	mv build/out/hodlcash-osx-signed.dmg ../hodlcash-${VERSION}-osx.dmg
	popd

  Create the signed Windows binaries:

	pushd ./gitian-builder
	./bin/gbuild -i --commit signature=v${VERSION} ../hodlcash/contrib/gitian-descriptors/gitian-win-signer.yml
	./bin/gsign --signer $SIGNER --release ${VERSION}-win-signed --destination ../gitian.sigs/ ../hodlcash/contrib/gitian-descriptors/gitian-win-signer.yml
	mv build/out/hodlcash-*win64-setup.exe ../hodlcash-${VERSION}-win64-setup.exe
	mv build/out/hodlcash-*win32-setup.exe ../hodlcash-${VERSION}-win32-setup.exe
	popd

Commit your signature for the signed OS X/Windows binaries:

	pushd gitian.sigs
	git add ${VERSION}-osx-signed/${SIGNER}
	git add ${VERSION}-win-signed/${SIGNER}
	git commit -a
	git push  # Assuming you can push to the gitian.sigs tree
	popd

-------------------------------------------------------------------------

### After 3 or more people have gitian-built and their results match:

- Create `SHA256SUMS.asc` for the builds, and GPG-sign it:
```bash
sha256sum * > SHA256SUMS
gpg --digest-algo sha256 --clearsign SHA256SUMS # outputs SHA256SUMS.asc
rm SHA256SUMS
```
(the digest algorithm is forced to sha256 to avoid confusion of the `Hash:` header that GPG adds with the SHA256 used for the files)
Note: check that SHA256SUMS itself doesn't end up in SHA256SUMS, which is a spurious/nonsensical entry.

- Upload zips and installers, as well as `SHA256SUMS.asc` from last step, to the hodlcash.com server
  into `/var/www/bin/hodlcash-core-${VERSION}`

- Update hodlcash.com version

  - First, check to see if the HodlCash.org maintainers have prepared a
    release: https://github.com/hodlcash/hodlcash.com/labels/Releases

      - If they have, it will have previously failed their Travis CI
        checks because the final release files weren't uploaded.
        Trigger a Travis CI rebuild---if it passes, merge.

  - If they have not prepared a release, follow the HodlCash.org release
    instructions: https://github.com/hodlcash/hodlcash.com#release-notes

  - After the pull request is merged, the website will automatically show the newest version within 15 minutes, as well
    as update the OS download links. Ping @saivann/@harding (saivann/harding on Freenode) in case anything goes wrong

- Announce the release:

  - Release sticky on hodlcashtalk: https://hodlcashtalk.org/index.php?board=1.0

  - HodlCash-development mailing list

  - Update title of #hodlcash on Freenode IRC

  - Optionally reddit /r/HodlCash, ... but this will usually sort out itself

- Notify BlueMatt so that he can start building [https://launchpad.net/~hodlcash/+archive/ubuntu/hodlcash](the PPAs)

- Add release notes for the new version to the directory `doc/release-notes` in git master

- Celebrate
