var mediaWikiLoadStart=(new Date()).getTime();function isCompatible(ua){if(ua===undefined){ua=navigator.userAgent;}return!((ua.indexOf('MSIE')!==-1&&parseFloat(ua.split('MSIE')[1])<8)||(ua.indexOf('Firefox/')!==-1&&parseFloat(ua.split('Firefox/')[1])<3)||(ua.indexOf('Opera/')!==-1&&(ua.indexOf('Version/')===-1?parseFloat(ua.split('Opera/')[1])<10:parseFloat(ua.split('Version/')[1])<12))||(ua.indexOf('Opera ')!==-1&&parseFloat(ua.split(' Opera ')[1])<10)||ua.match(/BlackBerry[^\/]*\/[1-5]\./)||ua.match(/webOS\/1\.[0-4]/)||ua.match(/PlayStation/i)||ua.match(/SymbianOS|Series60/)||ua.match(/NetFront/)||ua.match(/Opera Mini/)||ua.match(/S40OviBrowser/)||(ua.match(/Glass/)&&ua.match(/Android/)));}var startUp=function(){mw.config=new mw.Map(true);mw.loader.addSource({"local":"/wiki/load.php"});mw.loader.register([["site",1466508162,[],"site"],["noscript",1466508162,[],"noscript"],["filepage",1466508162],["user.groups",1466508162,[],"user"],["user",1466508162,[],"user"],["user.cssprefs",
1466508162,[],"private"],["user.defaults",1466508162],["user.options",1466508162,[6],"private"],["user.tokens",1466508162,[],"private"],["mediawiki.language.data",1466508162,[149]],["mediawiki.skinning.elements",1466508162],["mediawiki.skinning.content",1466508162],["mediawiki.skinning.interface",1466508162],["mediawiki.skinning.content.parsoid",1466508162],["mediawiki.skinning.content.externallinks",1466508162],["jquery.accessKeyLabel",1468811407,[25,43]],["jquery.appear",1466508162],["jquery.arrowSteps",1466508162],["jquery.async",1466508162],["jquery.autoEllipsis",1466508162,[37]],["jquery.badge",1466508162,[146]],["jquery.byteLength",1466508162],["jquery.byteLimit",1466508162,[21]],["jquery.checkboxShiftClick",1466508162],["jquery.chosen",1466508162],["jquery.client",1466508162],["jquery.color",1466508162,[27]],["jquery.colorUtil",1466508162],["jquery.confirmable",1466508162,[150]],["jquery.cookie",1466508162],["jquery.expandableField",1466508162],["jquery.farbtastic",1466508162,[
27]],["jquery.footHovzer",1466508162],["jquery.form",1466508162],["jquery.fullscreen",1466508162],["jquery.getAttrs",1466508162],["jquery.hidpi",1466508162],["jquery.highlightText",1466508162,[43]],["jquery.hoverIntent",1466508162],["jquery.localize",1466508162],["jquery.makeCollapsible",1468810827],["jquery.mockjax",1466508162],["jquery.mw-jump",1466508162],["jquery.mwExtension",1466508162],["jquery.placeholder",1466508162],["jquery.qunit",1466508162],["jquery.qunit.completenessTest",1466508162,[45]],["jquery.spinner",1466508162],["jquery.jStorage",1466508162,[91]],["jquery.suggestions",1466508162,[37]],["jquery.tabIndex",1466508162],["jquery.tablesorter",1491839619,[43,151]],["jquery.textSelection",1466508162,[25]],["jquery.throttle-debounce",1466508162],["jquery.validate",1466508162],["jquery.xmldom",1466508162],["jquery.tipsy",1466508162],["jquery.ui.core",1466508162,[],"jquery.ui"],["jquery.ui.accordion",1466508162,[57,76],"jquery.ui"],["jquery.ui.autocomplete",1466508162,[65],
"jquery.ui"],["jquery.ui.button",1466508162,[57,76],"jquery.ui"],["jquery.ui.datepicker",1466508162,[57],"jquery.ui"],["jquery.ui.dialog",1466508162,[60,63,67,69],"jquery.ui"],["jquery.ui.draggable",1466508162,[57,66],"jquery.ui"],["jquery.ui.droppable",1466508162,[63],"jquery.ui"],["jquery.ui.menu",1466508162,[57,67,76],"jquery.ui"],["jquery.ui.mouse",1466508162,[76],"jquery.ui"],["jquery.ui.position",1466508162,[],"jquery.ui"],["jquery.ui.progressbar",1466508162,[57,76],"jquery.ui"],["jquery.ui.resizable",1466508162,[57,66],"jquery.ui"],["jquery.ui.selectable",1466508162,[57,66],"jquery.ui"],["jquery.ui.slider",1466508162,[57,66],"jquery.ui"],["jquery.ui.sortable",1466508162,[57,66],"jquery.ui"],["jquery.ui.spinner",1466508162,[60],"jquery.ui"],["jquery.ui.tabs",1466508162,[57,76],"jquery.ui"],["jquery.ui.tooltip",1466508162,[57,67,76],"jquery.ui"],["jquery.ui.widget",1466508162,[],"jquery.ui"],["jquery.effects.core",1466508162,[],"jquery.ui"],["jquery.effects.blind",1466508162,[77],
"jquery.ui"],["jquery.effects.bounce",1466508162,[77],"jquery.ui"],["jquery.effects.clip",1466508162,[77],"jquery.ui"],["jquery.effects.drop",1466508162,[77],"jquery.ui"],["jquery.effects.explode",1466508162,[77],"jquery.ui"],["jquery.effects.fade",1466508162,[77],"jquery.ui"],["jquery.effects.fold",1466508162,[77],"jquery.ui"],["jquery.effects.highlight",1466508162,[77],"jquery.ui"],["jquery.effects.pulsate",1466508162,[77],"jquery.ui"],["jquery.effects.scale",1466508162,[77],"jquery.ui"],["jquery.effects.shake",1466508162,[77],"jquery.ui"],["jquery.effects.slide",1466508162,[77],"jquery.ui"],["jquery.effects.transfer",1466508162,[77],"jquery.ui"],["json",1466508162,[],null,null,"return!!(window.JSON\u0026\u0026JSON.stringify\u0026\u0026JSON.parse);"],["moment",1466508162],["mediawiki.apihelp",1466508162,[112]],["mediawiki.template",1466508162],["mediawiki.template.mustache",1466508162],["mediawiki.apipretty",1466508162],["mediawiki.api",1466508162,[128]],["mediawiki.api.category",
1466508162,[123,97]],["mediawiki.api.edit",1466508162,[123,97,8]],["mediawiki.api.login",1466508162,[97]],["mediawiki.api.options",1466508162,[97]],["mediawiki.api.parse",1466508162,[97]],["mediawiki.api.watch",1466508162,[97,8]],["mediawiki.content.json",1466508162],["mediawiki.confirmCloseWindow",1466508162],["mediawiki.debug",1466508162,[32,56]],["mediawiki.debug.init",1466508162,[106]],["mediawiki.feedback",1466508162,[123,116,204]],["mediawiki.filewarning",1466508162,[204]],["mediawiki.helplink",1466508162],["mediawiki.hidpi",1466508162,[36],null,null,"return'srcset'in new Image();"],["mediawiki.hlist",1466508162,[25]],["mediawiki.htmlform",1475347774,[22,43]],["mediawiki.icon",1466508162],["mediawiki.inspect",1466508162,[21,91]],["mediawiki.messagePoster",1466508162,[97,203]],["mediawiki.messagePoster.wikitext",1466508162,[99,116]],["mediawiki.notification",1466508162,[157]],["mediawiki.notify",1466508162],["mediawiki.pager.tablePager",1466508162],["mediawiki.searchSuggest",
1468810827,[35,44,49,97]],["mediawiki.sectionAnchor",1466508162],["mediawiki.Title",1466508162,[21,128]],["mediawiki.toc",1468810969,[29]],["mediawiki.Uri",1466508162,[128]],["mediawiki.user",1466508162,[29,97,7,8]],["mediawiki.userSuggest",1466508162,[49,97]],["mediawiki.util",1466508162,[15,119]],["mediawiki.cookie",1466508162,[29]],["mediawiki.toolbar",1466508162],["mediawiki.action.edit",1466508162,[22,52,132]],["mediawiki.action.edit.styles",1466508162],["mediawiki.action.edit.collapsibleFooter",1466508162,[29,40,114]],["mediawiki.action.edit.preview",1466508162,[33,47,52,137,97,150]],["mediawiki.action.edit.stash",1466508162,[35,97]],["mediawiki.action.history",1466508162,[],"mediawiki.action.history"],["mediawiki.action.history.diff",1466508162,[],"mediawiki.action.history"],["mediawiki.action.view.dblClickEdit",1466508162,[157,7]],["mediawiki.action.view.metadata",1469424924],["mediawiki.action.view.categoryPage.styles",1466508162],["mediawiki.action.view.postEdit",1468811408,[
129,150,94]],["mediawiki.action.view.redirect",1466508162,[25]],["mediawiki.action.view.redirectPage",1466508162],["mediawiki.action.view.rightClickEdit",1466508162],["mediawiki.action.edit.editWarning",1468898231,[52,105,150]],["mediawiki.language",1468811408,[147,9]],["mediawiki.cldr",1466508162,[148]],["mediawiki.libs.pluralruleparser",1466508162],["mediawiki.language.init",1466508162],["mediawiki.jqueryMsg",1466508162,[146,128]],["mediawiki.language.months",1491839619,[146]],["mediawiki.language.names",1466508162,[149]],["mediawiki.language.specialCharacters",1466508162,[146]],["mediawiki.libs.jpegmeta",1466508162],["mediawiki.page.gallery",1466508162,[53]],["mediawiki.page.ready",1466508162,[15,23,40,42,44]],["mediawiki.page.startup",1466508162,[128]],["mediawiki.page.patrol.ajax",1466508162,[47,123,97,157,8]],["mediawiki.page.watch.ajax",1468824900,[103,157]],["mediawiki.page.image.pagination",1466508162,[47,125]],["mediawiki.special",1466508162],["mediawiki.special.block",
1466508162,[128]],["mediawiki.special.changeemail",1466508162,[128]],["mediawiki.special.changeslist",1466508162],["mediawiki.special.changeslist.legend",1466508162],["mediawiki.special.changeslist.legend.js",1466508162,[29,40]],["mediawiki.special.changeslist.enhanced",1466508162],["mediawiki.special.edittags",1466508162,[24]],["mediawiki.special.edittags.styles",1466508162],["mediawiki.special.import",1466508162],["mediawiki.special.movePage",1466508162,[22]],["mediawiki.special.pageLanguage",1466508162],["mediawiki.special.pagesWithProp",1466508162],["mediawiki.special.preferences",1484831146,[105,146]],["mediawiki.special.recentchanges",1466508162,[161]],["mediawiki.special.search",1468851804],["mediawiki.special.undelete",1466508162],["mediawiki.special.upload",1484600115,[47,123,97,105,150,154,94]],["mediawiki.special.userlogin.common.styles",1466508162],["mediawiki.special.userlogin.signup.styles",1466508162],["mediawiki.special.userlogin.login.styles",1466508162],[
"mediawiki.special.userlogin.common.js",1468810827],["mediawiki.special.userlogin.signup.js",1466508162,[53,97,150]],["mediawiki.special.unwatchedPages",1466508162,[123,103]],["mediawiki.special.javaScriptTest",1466508162,[125]],["mediawiki.special.version",1466508162],["mediawiki.legacy.config",1466508162],["mediawiki.legacy.ajax",1466508162,[193]],["mediawiki.legacy.commonPrint",1466508162],["mediawiki.legacy.protect",1466508162,[22]],["mediawiki.legacy.shared",1466508162],["mediawiki.legacy.oldshared",1466508162],["mediawiki.legacy.wikibits",1466508162,[128]],["mediawiki.ui",1466508162],["mediawiki.ui.checkbox",1466508162],["mediawiki.ui.radio",1466508162],["mediawiki.ui.anchor",1466508162],["mediawiki.ui.button",1466508162],["mediawiki.ui.input",1466508162],["mediawiki.ui.icon",1466508162],["mediawiki.ui.text",1466508162],["es5-shim",1466508162,[],null,null,"return(function(){'use strict';return!this\u0026\u0026!!Function.prototype.bind;}());"],["oojs",1466508162,[202,91]],[
"oojs-ui",1466508162,[203,205]],["oojs-ui.styles",1466508162],["skins.cologneblue",1466508162],["skins.modern",1466508162],["skins.monobook.styles",1466508162],["skins.vector.styles",1466508162],["skins.vector.js",1466508162,[50,53]]]);mw.config.set({"wgLoadScript":"/wiki/load.php","debug":false,"skin":"monobook","stylepath":"/wiki/skins","wgUrlProtocols":"bitcoin\\:|ftp\\:\\/\\/|ftps\\:\\/\\/|geo\\:|git\\:\\/\\/|gopher\\:\\/\\/|http\\:\\/\\/|https\\:\\/\\/|irc\\:\\/\\/|ircs\\:\\/\\/|magnet\\:|mailto\\:|mms\\:\\/\\/|news\\:|nntp\\:\\/\\/|redis\\:\\/\\/|sftp\\:\\/\\/|sip\\:|sips\\:|sms\\:|ssh\\:\\/\\/|svn\\:\\/\\/|tel\\:|telnet\\:\\/\\/|urn\\:|worldwind\\:\\/\\/|xmpp\\:|\\/\\/","wgArticlePath":"/wiki/articles/$1","wgScriptPath":"/wiki","wgScriptExtension":".php","wgScript":"/wiki/index.php","wgSearchType":null,"wgVariantArticlePath":false,"wgActionPaths":{},"wgServer":"http://gbdev.gg8.se","wgServerName":"gbdev.gg8.se","wgUserLanguage":"en","wgContentLanguage":"en","wgTranslateNumerals"
:true,"wgVersion":"1.25.1","wgEnableAPI":true,"wgEnableWriteAPI":true,"wgMainPageTitle":"Main Page","wgFormattedNamespaces":{"-2":"Media","-1":"Special","0":"","1":"Talk","2":"User","3":"User talk","4":"GbdevWiki","5":"GbdevWiki talk","6":"File","7":"File talk","8":"MediaWiki","9":"MediaWiki talk","10":"Template","11":"Template talk","12":"Help","13":"Help talk","14":"Category","15":"Category talk"},"wgNamespaceIds":{"media":-2,"special":-1,"":0,"talk":1,"user":2,"user_talk":3,"gbdevwiki":4,"gbdevwiki_talk":5,"file":6,"file_talk":7,"mediawiki":8,"mediawiki_talk":9,"template":10,"template_talk":11,"help":12,"help_talk":13,"category":14,"category_talk":15,"image":6,"image_talk":7,"project":4,"project_talk":5},"wgContentNamespaces":[0],"wgSiteName":"GbdevWiki","wgDBname":"gbdevwiki","wgAvailableSkins":{"cologneblue":"CologneBlue","modern":"Modern","monobook":"MonoBook","vector":"Vector","fallback":"Fallback","apioutput":"ApiOutput"},"wgExtensionAssetsPath":"/wiki/extensions",
"wgCookiePrefix":"gbdevwiki_gbdevwiki_","wgCookieDomain":"","wgCookiePath":"/","wgCookieExpiration":15552000,"wgResourceLoaderMaxQueryLength":2000,"wgCaseSensitiveNamespaces":[],"wgLegalTitleChars":" %!\"$\u0026'()*,\\-./0-9:;=?@A-Z\\\\\\^_`a-z~+\\u0080-\\uFFFF","wgResourceLoaderStorageVersion":1,"wgResourceLoaderStorageEnabled":false});};if(isCompatible()){document.write("\u003Cscript src=\"/wiki/load.php?debug=false\u0026amp;lang=en\u0026amp;modules=jquery%2Cmediawiki\u0026amp;only=scripts\u0026amp;skin=monobook\u0026amp;version=20160413T062643Z\"\u003E\u003C/script\u003E");};
/* cache key: gbdevwiki-gbdevwiki_:resourceloader:filter:minify-js:7:37b4be764dfa468677016d951b22f681 */