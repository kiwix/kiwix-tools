// Terminology
//
// user url: identifier of the page that has to be displayed in the viewer
//           and that is used as the hash component of the viewer URL. For
//           book resources the user url is {book}/{resource} .
//
// iframe url: the URL to be loaded in the viewer iframe.

let viewerState = {
  uiLanguage: 'en',
};

const FULL_ROOT_URL = `${window.location.origin}${root}`;

function dropUserLang(query) {
  const q = new URLSearchParams(query);
  q.delete('userlang');
  const pre = (query.startsWith('?') && q.size != 0 ? '?' : '');
  return pre + q.toString();
}

function userUrl2IframeUrl(url) {
  if ( url == '' ) {
    return blankPageUrl;
  }

  if ( url.startsWith('search?') ) {
    const q = new URLSearchParams(url.slice("search?".length));
    q.set('userlang', viewerState.uiLanguage);
    return `${root}/search?${q.toString()}`;
  }

  return `${root}/content/${url}`;
}

function getBookFromUserUrl(url) {
  if ( url == '' ) {
    return null;
  }

  if ( url.startsWith('search?') ) {
    const p = new URLSearchParams(url.slice("search?".length));
    return p.get('books.name') || p.get('content');
  }
  return url.split('/')[0];
}

let currentBook = null;
let currentBookTitle = null;

const bookUIGroup = document.getElementById('kiwix_serve_taskbar_book_ui_group');
const homeButton = document.getElementById('kiwix_serve_taskbar_home_button');
const contentIframe = document.getElementById('content_iframe');


function gotoMainPageOfCurrentBook() {
  location.hash = currentBook + '/';
}

function gotoUrl(url) {
  contentIframe.src = root + url;
}

function gotoRandomPage() {
  gotoUrl(`/random?content=${currentBook}`);
}

// URI-encodes only the specified special symbols (note, however, that '%' is
// always considered a special symbol).
function quasiUriEncode(s, specialSymbols) {
  if ( specialSymbols.match(/[A-Za-z0-9]/) ) {
    throw "Alphanumeric symbols cannot be special";
  }

  // Set's guarantee of iterating in insertion order ensures that
  // all %s in s will be encoded first.
  for ( const c of new Set('%' + specialSymbols) ) {
    s = s.replaceAll(c, encodeURIComponent(c));
  }

  return s;
}

function performSearch() {
  const searchbox = document.getElementById('kiwixsearchbox');
  if (!searchbox.value.trim()) { return;}
  const q = encodeURIComponent(searchbox.value);
  gotoUrl(`/search?books.name=${currentBook}&pattern=${q}&userlang=${viewerState.uiLanguage}`);
}

function makeJSLink(jsCodeString, linkText, linkAttr="") {
  // Values of the href attribute are assumed by the browser to be
  // fully URI-encoded (no matter what the scheme is). Therefore, in
  // order to prevent the browser from decoding any URI-encoded parts
  // in the JS code we have to URI-encode a second time.
  // (see https://stackoverflow.com/questions/33721510)
  const uriEncodedJSCode = encodeURIComponent(jsCodeString);
  const linkPlainText = htmlDecode(linkText, "text/html");
  linkAttr += ` href="javascript:${uriEncodedJSCode}"`;
  linkAttr += ` title="${linkPlainText}"`;
  return `<a ${linkAttr}>${linkText}</a>`;
}

function suggestionsApiURL()
{
  const uriEncodedBookName = encodeURIComponent(currentBook);
  const userLang = viewerState.uiLanguage;
  return `${root}/suggest?userlang=${userLang}&content=${uriEncodedBookName}`;
}

function setTitle(element, text) {
  if ( element ) {
    element.title = text;
    if ( element.hasAttribute("aria-label") ) {
      element.setAttribute("aria-label", text);
    }
  }
}

function setCurrentBook(book, title) {
  currentBook = book;
  currentBookTitle = title;
  setTitle(homeButton, $t("home-button-text", {BOOK_TITLE: title}));
  homeButton.innerHTML = `<button>${title}</button>`;
  bookUIGroup.style.display = 'inline';
  updateSearchBoxForBookChange();
}

function noCurrentBook() {
  currentBook = null;
  currentBookTitle = null;
  bookUIGroup.style.display = 'none';
  updateSearchBoxForBookChange();
}

function updateCurrentBookIfNeeded(userUrl) {
  const book = getBookFromUserUrl(userUrl);
  if ( currentBook != book ) {
    updateCurrentBook(book);
  }
}

function updateCurrentBook(book) {
  if ( book == null ) {
    noCurrentBook();
  } else {
    fetch(`./raw/${book}/meta/Title`).then(async (resp) => {
      if ( resp.ok ) {
        setCurrentBook(book, await resp.text());
      } else {
        noCurrentBook();
      }
    }).catch((err) => {
      console.log("Error fetching book title: " + err);
      noCurrentBook();
    });
  }
}

function iframeUrl2UserUrl(url, query) {
  if ( url == blankPageUrl ) {
    return '';
  }

  if ( url == `${root}/search` ) {
    return `search${dropUserLang(query)}`;
  }

  url = url.slice(root.length);

  return url.split('/').slice(2).join('/');
}

function getSearchPattern() {
  const url = window.location.hash.slice(1);
  if ( url.startsWith('search?') ) {
    const p = new URLSearchParams(url.slice("search?".length));
    return p.get("pattern");
  }
  return null;
}


let autoCompleteJS = null;

function closeSuggestions() {
  if ( autoCompleteJS ) {
    autoCompleteJS.close();
  }
}

function updateSearchBoxForLocationChange() {
  closeSuggestions();
  document.getElementById("kiwixsearchbox").value = getSearchPattern();
}

function updateSearchBoxForBookChange() {
  const searchbox = document.getElementById('kiwixsearchbox');
  const kiwixSearchFormWrapper = document.querySelector('.kiwix_searchform');
  if ( currentBookTitle ) {
    searchbox.title = $t("searchbox-tooltip", {BOOK_TITLE : currentBookTitle});
    searchbox.placeholder = searchbox.title;
    searchbox.setAttribute("aria-label", searchbox.title);
    kiwixSearchFormWrapper.style.display = 'inline';
  } else {
    kiwixSearchFormWrapper.style.display = 'none';
  }
}

let previousScrollTop = Infinity;

function updateToolbarVisibilityState() {
  const iframeDoc = contentIframe.contentDocument;
  const st = iframeDoc.documentElement.scrollTop || iframeDoc.body.scrollTop;
  if ( Math.abs(previousScrollTop - st) <= 5 )
      return;

  const kiwixToolBar = document.querySelector('#kiwixtoolbar');

  if (st > previousScrollTop) {
      kiwixToolBar.style.position = 'fixed';
      kiwixToolBar.style.top = '-100%';
  } else {
      kiwixToolBar.style.position = 'static';
      kiwixToolBar.style.top = '0';
  }

  previousScrollTop = st;
}

function handle_visual_viewport_change() {
  const wh = window.visualViewport
           ? window.visualViewport.height
           : window.innerHeight;
  contentIframe.height = wh - contentIframe.offsetTop - 4;
}

function setIframeUrl(path) {
  try {
    // Don't do anything if we are already at the requested URL.
    // This is needed to break the infinite ping-pong played by
    // handle_location_hash_change() and handle_content_url_change()
    // (when either top-window or content window/iframe URL changes the other
    // one is updated too).
    if ( path == contentIframe.contentWindow.location.pathname )
      return;
  } catch ( error ) {
    // This happens in Firefox when a PDF file is displayed in the iframe
    // (sandboxing of the iframe content and cross-origin mismatch with the
    // builtin PDF viewer result in preventing access to the attributes of
    // contentIframe.contentWindow.location).
    // Fall through and load the requested URL.
  }
  contentIframe.contentWindow.location.replace(path);
}

function handle_location_hash_change() {
  const hash = window.location.hash.slice(1);
  console.log("handle_location_hash_change: " + hash);
  updateCurrentBookIfNeeded(hash);
  setIframeUrl(userUrl2IframeUrl(hash));
  updateSearchBoxForLocationChange();
  previousScrollTop = Infinity;
  history.replaceState(viewerState, null);
}

function translateErrorPageIfNeeded() {
  translatePageInWindow(contentIframe.contentWindow);
}


let iframeLocationHref = null;

function handle_content_url_change() {
  const iframeLocation = contentIframe.contentWindow.location;

  if ( iframeLocationHref == iframeLocation.href ||
       !iframeLocation.pathname.startsWith(root + '/content/') )
    return;

  iframeLocationHref = iframeLocation.href;
  console.log('handle_content_url_change: ' + iframeLocation.href);
  document.title = contentIframe.contentDocument.title;
  const iframeContentUrl = iframeLocation.pathname + iframeLocation.hash;
  const iframeContentQuery = iframeLocation.search;
  const newHash = iframeUrl2UserUrl(iframeContentUrl, iframeContentQuery);
  history.replaceState(viewerState, null, makeURL(location.search, newHash));
  updateCurrentBookIfNeeded(newHash);
  translateErrorPageIfNeeded();
};

////////////////////////////////////////////////////////////////////////////////
// External link blocking
////////////////////////////////////////////////////////////////////////////////

function matchingAncestorElement(el, context, selector) {
  while (el && el.matches && el !== context) {
    if ( el.matches(selector) )
      return el;
    el = el.parentElement;
  }
  return null;
}

const block_path = `${root}/catch/external`;

function blockLink(url) {
  return viewerSettings.linkBlockingEnabled
       ? block_path + "?source=" + encodeURIComponent(url)
       : url;
}

function urlMustBeHandledByAnExternalApp(url) {
  const WHITELISTED_URL_SCHEMATA = ['http:', 'https:', 'about:', 'javascript:'];

  return WHITELISTED_URL_SCHEMATA.indexOf(url.protocol) == -1;
}

function isExternalUrl(url) {
  if ( url.startsWith(window.location.origin) )
    return false;

  return url.startsWith("//")
      || url.startsWith("http:")
      || url.startsWith("https:");
}

function getRealHref(target) {
  // In case of wombat in the middle, wombat will rewrite the href value to the original url (external link)
  // This is not what we want. Let's ask wombat to not rewrite href
  const old_no_rewrite = target._no_rewrite;
  target._no_rewrite = true;
  const target_href = target.href;
  target._no_rewrite = old_no_rewrite;
  return target_href;
}

function setHrefAvoidingWombatRewriting(target, url) {
  const old_no_rewrite = target._no_rewrite;
  target._no_rewrite = true;
  target.setAttribute("href", url);
  target._no_rewrite = old_no_rewrite;
}

function linkShouldBeOpenedInANewWindow(linkElement, mouseEvent) {
  return linkElement.getAttribute("target") == "_blank"
      || mouseEvent.ctrlKey
      || mouseEvent.shiftKey
      || mouseEvent.metaKey /* on Macs */;
}

function goingToOpenALinkToAnUndisplayableResource(url) {
  return !navigator.pdfViewerEnabled && url.pathname.endsWith('.pdf');
}

function onClickEvent(e) {
  const iframeDocument = contentIframe.contentDocument;
  const target = matchingAncestorElement(e.target, iframeDocument, "a");
  if (target !== null && "href" in target) {
    const target_href = getRealHref(target);
    const target_url = new URL(target_href, iframeDocument.location);
    if ( target_url.href.startsWith(`${FULL_ROOT_URL}/viewer#`) &&
         !linkShouldBeOpenedInANewWindow(target, e) ) {
      contentIframe.contentWindow.parent.location = target_url;
      e.preventDefault();
      return;
    }
    const isExternalAppUrl = urlMustBeHandledByAnExternalApp(target_url);
    if ( (isExternalAppUrl && !viewerSettings.linkBlockingEnabled)
         || goingToOpenALinkToAnUndisplayableResource(target_url) ) {
        target.setAttribute("target", "_blank");
    }

    if (isExternalAppUrl || isExternalUrl(target_href)) {
      const possiblyBlockedLink = blockLink(target_href);
      if ( linkShouldBeOpenedInANewWindow(target, e) ) {
        // The link will be loaded in a new tab/window - update the link
        // and let the browser handle the rest.
        setHrefAvoidingWombatRewriting(target, possiblyBlockedLink);
      } else {
        // Load the external URL in the viewer window (rather than iframe)
        contentIframe.contentWindow.parent.location = possiblyBlockedLink;
        e.preventDefault();
      }
    }
  }
}

// helper for enabling IE 8 event bindings
function addEventHandler(el, eventType, handler) {
  if (el.attachEvent)
    el.attachEvent('on'+eventType, handler);
  else
    el.addEventListener(eventType, handler);
}

function setupEventHandler(context, selector, eventType, callback) {
  addEventHandler(context, eventType, function(e) {
    const eventElement = e.target || e.srcElement;
    const el = matchingAncestorElement(eventElement, context, selector);
    if (el)
      callback.call(el, e);
  });
}

// matches polyfill
this.Element && function(ElementPrototype) {
    ElementPrototype.matches = ElementPrototype.matches ||
    ElementPrototype.matchesSelector ||
    ElementPrototype.webkitMatchesSelector ||
    ElementPrototype.msMatchesSelector ||
    function(selector) {
        var node = this, nodes = (node.parentNode || node.document).querySelectorAll(selector), i = -1;
        while (nodes[++i] && nodes[i] != node);
        return !!nodes[i];
    }
}(Element.prototype);

////////////////////////////////////////////////////////////////////////////////
// End of external link blocking
////////////////////////////////////////////////////////////////////////////////

const internalUrlPrefix = `${FULL_ROOT_URL}/content/`;

function setup_chaperon_mode() {
  setupEventHandler(contentIframe.contentDocument, 'a', 'click', onClickEvent);
  const links = contentIframe.contentDocument.getElementsByTagName('a');
  for ( const a of links ) {
    // XXX: wombat's possible involvement with href not taken into account
    if ( a.hasAttribute('href') && a.href.startsWith(internalUrlPrefix) ) {
      const userUrl = a.href.substr(internalUrlPrefix.length);
      a.href = `${root}/viewer#${userUrl}`;
    }
  }
}

function on_content_load() {
  const loader = document.getElementById("kiwix__loader");

  contentIframe.classList.remove("hidden");
  loader.style.display = "none";
  contentIframe.contentWindow.onhashchange = handle_content_url_change;
  setInterval(handle_content_url_change, 100);
  setup_chaperon_mode();
}

function htmlDecode(input) {
    var doc = new DOMParser().parseFromString(input, "text/html");
    return doc.documentElement.textContent;
}

function setupAutoHidingOfTheToolbar() {
  setInterval(updateToolbarVisibilityState, 250);
}

function setupSuggestions() {
  const kiwixSearchBox = document.querySelector('#kiwixsearchbox');
  const kiwixSearchFormWrapper = document.querySelector('.kiwix_searchform');

  autoCompleteJS = new autoComplete(
    {
      selector: "#kiwixsearchbox",
      placeHolder: kiwixSearchBox.title,
      threshold: 1,
      debounce: 300,
      data : {
        src: async (query) => {
          try {
            // Fetch Data from external Source
            const source = await fetch(`${suggestionsApiURL()}&term=${encodeURIComponent(query)}`);
            const data = await source.json();
            return data;
          } catch (error) {
            return error;
          }
        },
        keys: ['label'],
      },
      submit: true,
      searchEngine: (query, record) => {
        // We accept all records
        return true;
      },
      resultsList: {
        noResults: true,
        // We must display 10 results (requested) + 1 potential link to do a full text search.
        maxResults: 11,
      },
      resultItem: {
        element: (item, data) => {
          const uriEncodedBookName = encodeURIComponent(currentBook);
          let url;
          if (data.value.kind == "path") {
            // The double quote and backslash symbols are included in the list
            // of special symbols to URI-encode so that the resulting URL can
            // be safely quoted inside a dynamically executed piece of
            // Javascript code a few lines later.
            const path = htmlDecode(data.value.path);
            const quasiUriEncodedPath = quasiUriEncode(path, '#?"\\');
            url = `/content/${uriEncodedBookName}/${quasiUriEncodedPath}`;
          } else {
            const pattern = encodeURIComponent(htmlDecode(data.value.value));
            url = `/search?content=${uriEncodedBookName}&pattern=${pattern}`;
          }
          // url can't contain any double quote and/or backslash symbols
          // since they should have been URI-encoded. Therefore putting it
          // inside double quotes should result in valid javascript.
          const jsAction = `gotoUrl("${url}")`;
          const linkText = htmlDecode(data.value.label);
          item.innerHTML = makeJSLink(jsAction, linkText, 'class="suggest"');
        },
        highlight: "autoComplete_highlight",
        selected: "autoComplete_selected"
      }
    }
  );

  document.querySelector('#kiwixsearchform').addEventListener('submit', function(event) {
    closeSuggestions();
    try {
      const selectedElem = document.querySelector('.autoComplete_selected > a');
      if (selectedElem) {
        event.preventDefault();
        selectedElem.click();
      }
    } catch (err) {}
  });

  kiwixSearchBox.addEventListener('focus', () => {
      kiwixSearchFormWrapper.classList.add('full_width');
      document.querySelector('label[for="kiwix_button_show_toggle"]').classList.add('searching');
      document.querySelector('.kiwix_button_cont').classList.add('searching');
  });
  kiwixSearchBox.addEventListener('blur', () => {
      kiwixSearchFormWrapper.classList.remove('full_width');
      document.querySelector('label[for="kiwix_button_show_toggle"]').classList.remove('searching');
      document.querySelector('.kiwix_button_cont').classList.remove('searching');
  });
}

function makeURL(search, hash) {
  let url = location.origin + location.pathname;
  if (search != "") {
    url += (search[0] == '?' ? search : '?' + search);
  }

  url += (hash[0] == '#' ? hash : '#' + hash);
  return url;
}

function updateUILanguageSelector(userLang) {
  console.log(`updateUILanguageSelector(${userLang})`);
  const languageSelector = document.getElementById("ui_language");
  for (const opt of languageSelector.children ) {
    if ( opt.value == userLang ) {
      opt.selected = true;
    }
  }
}

function handle_history_state_change(event) {
  console.log(`handle_history_state_change`);
  if ( event.state ) {
    viewerState = event.state;
    updateUILanguageSelector(viewerState.uiLanguage);
    setUserLanguage(viewerState.uiLanguage, updateUIText);
  }
}

function changeUILanguage() {
  window.modalUILanguageSelector.close();
  const s = document.getElementById("ui_language");
  const lang = s.options[s.selectedIndex].value;
  viewerState.uiLanguage = lang;
  setUserLanguage(lang, () => {
    updateUIText();
    translateErrorPageIfNeeded();
    history.pushState(viewerState, null);
  });
}

function setupViewer() {
  // Defer the call of handle_visual_viewport_change() until after the
  // presence or absence of the taskbar as determined by this function
  // has been settled.
  setTimeout(handle_visual_viewport_change, 0);

  window.onresize = handle_visual_viewport_change;

  const kiwixToolBarWrapper = document.getElementById('kiwixtoolbarwrapper');
  if ( ! viewerSettings.toolbarEnabled ) {
    finishViewerSetup();
    return;
  }

  const lang = getUserLanguage();
  setUserLanguage(lang, finishViewerSetupOnceTranslationsAreLoaded);
  viewerState.uiLanguage = lang;
  const cleanedUpQuery = dropUserLang(window.location.search);
  const rewrittenURL = makeURL(cleanedUpQuery, location.hash);
  history.replaceState(viewerState, null, rewrittenURL);

  kiwixToolBarWrapper.style.display = 'block';
  if ( ! viewerSettings.libraryButtonEnabled ) {
    document.getElementById("kiwix_serve_taskbar_library_button").remove();
  }

  initUILanguageSelector(viewerState.uiLanguage, changeUILanguage);
  setupSuggestions();

  // cybook hack
  if (navigator.userAgent.indexOf("bookeen/cybook") != -1) {
      document.querySelector('html').classList.add('cybook');
  }

  if (document.body.clientWidth < 520) {
    setupAutoHidingOfTheToolbar();
  }
}

function updateUIText() {
  currentBook = getBookFromUserUrl(location.hash.slice(1));
  updateCurrentBook(currentBook);

  setTitle(document.getElementById("kiwix_serve_taskbar_library_button"),
           $t("library-button-text"));

  setTitle(document.getElementById("kiwix_serve_taskbar_random_button"),
           $t("random-page-button-text"));

  // Add translation for loading text as soon as translations are available
  const loadingTextElement = document.getElementById("kiwix__loading_text");
  if (loadingTextElement) {
    loadingTextElement.textContent = $t("text-loading-content");
  }
}

function finishViewerSetupOnceTranslationsAreLoaded()
{
  updateUIText();
  finishViewerSetup();
}

function finishViewerSetup()
{
  handle_location_hash_change();

  window.onhashchange = handle_location_hash_change;
  window.onpopstate = handle_history_state_change;
}
