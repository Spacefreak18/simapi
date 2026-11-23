# Changelog

## [0.4.0](https://github.com/Spacefreak18/simapi/compare/0.3.1...0.4.0) (2026-11-23)


### Features

* **simd:** keep bridged telemetry across a restart  ([#49](https://github.com/Spacefreak18/simapi/issues/49)) ([f5d462e](https://github.com/Spacefreak18/simapi/commit/f5d462e6bde049da42f0b295120fbe8505f11582))


### Bug Fixes

* stop pidof() leaking /proc handles on every match ([#48](https://github.com/Spacefreak18/simapi/issues/48)) ([58ef123](https://github.com/Spacefreak18/simapi/commit/58ef1234a9fe0167f99417f9afb9595d9426438d))
* update include directory after moving code ([3fc9d39](https://github.com/Spacefreak18/simapi/commit/3fc9d396fcd30b1f7dcad34a322e79ec24c863fd))

## [0.3.1](https://github.com/Spacefreak18/simapi/compare/0.3.0...0.3.1) (2026-11-20)


### Bug Fixes

* uninitialized SimMap state causing crashes with simd ([#43](https://github.com/Spacefreak18/simapi/issues/43)) ([afb06ff](https://github.com/Spacefreak18/simapi/commit/afb06ff175b3c076f8b3bef57fb3fe0201ecd969))

## [0.3.0](https://github.com/Spacefreak18/simapi/compare/0.2.2...0.3.0) (2026-11-20)


### Features

* mqtt capabilities ([85d5212](https://github.com/Spacefreak18/simapi/commit/85d5212031a6f942b04552b1e317f5c47a117192))


### Bug Fixes

* do not open sim files if they are zero sized ([92941c3](https://github.com/Spacefreak18/simapi/commit/92941c3b2287e47899445efbffd6304d3094e02b))
* mapapi not properly set so apps can see simd is running ([633a968](https://github.com/Spacefreak18/simapi/commit/633a968564252599badd0587c920c2e7976c89d8))
* set a car on each iteration so one is definitely set to avoid crashes. But can be overwritten by sim later ([14e5349](https://github.com/Spacefreak18/simapi/commit/14e534989df48fe40cf414a6ba1f56bbd7215699))
* typo causing some compat shared memory not to close ([4251038](https://github.com/Spacefreak18/simapi/commit/425103857f8128ca7395dffdd81c71c6843d6750))

## [0.2.2](https://github.com/Spacefreak18/simapi/compare/0.2.1...0.2.2) (2026-10-13)


### Bug Fixes

* fix flatpak release ([a622882](https://github.com/Spacefreak18/simapi/commit/a62288239bd15c2b6e70f539102d19656bf37e25))

## [0.2.1](https://github.com/Spacefreak18/simapi/compare/0.2.0...0.2.1) (2026-10-13)


### Bug Fixes

* fix log message spelling ([80e7798](https://github.com/Spacefreak18/simapi/commit/80e7798aa5a0beb68c00afb115f77064a416adb2))
* log message spelling ([1c54c8c](https://github.com/Spacefreak18/simapi/commit/1c54c8cf790ba71f7462f749609ff02a1ae14292))

## [0.2.0](https://github.com/Spacefreak18/simapi/compare/0.1.25...0.2.0) (2026-10-13)


### Features

* enable automatic bridge support for Le Mans Ultimate in simd ([#38](https://github.com/Spacefreak18/simapi/issues/38)) ([899d535](https://github.com/Spacefreak18/simapi/commit/899d535df031db308e44239ce962de8e1a3cacd5))


### Bug Fixes

* bootstrap version ([a52b16d](https://github.com/Spacefreak18/simapi/commit/a52b16d5b998c08d37bb0bd6da9911849c2d983d))
* cleaned log messages ([eb983fe](https://github.com/Spacefreak18/simapi/commit/eb983fe2f6332cb26c035a2b12870c17347c9fde))
* misleading log messages ([ffddfd2](https://github.com/Spacefreak18/simapi/commit/ffddfd264b41b1ac2aac1d297103850fde8bb0fc))
* temporarily remove nix packages ([4149cf9](https://github.com/Spacefreak18/simapi/commit/4149cf9d13fcb7a19aba470209488b5ceb6d07b6))
