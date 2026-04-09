(function() {
    class FragmentParams extends URLSearchParams {
        constructor(fragment = '') {
            if (fragment[0] == '#')
                fragment = fragment.substring(1);
            super(fragment);
        }
    }

    const root = document.querySelector(`link[type='root']`).getAttribute('href');
    const incrementalLoadingParams = {
        start: 0,
        count: viewPortToCount()
    };
    const bookOrderMap = new Map();
    const filterCookieName = 'filters';
    const oneDayDelta = 86400000;
    let loader;
    let footer;
    let fadeOutDiv;
    let iso;
    let isFetching = false;
    let noResultInjected = false;
    let filters = getCookie(filterCookieName);
    let params = new FragmentParams(window.location.hash || filters || '');
    params.delete('userlang');
    let timer;
    let languages = {};
    let previousScrollTop = Infinity;

    function updateFeedLink() {
        const inputParams = new FragmentParams(window.location.hash);
        const filteredParams = new FragmentParams();
        for (const [key, value] of inputParams) {
            if ( value != '' ) {
                filteredParams.set(key, value);
            }
        }
        const feedLink = `${root}/catalog/v2/entries?count=-1&${filteredParams.toString()}`;
        document.querySelector('#headFeedLink').href = feedLink;
        document.querySelector('#feedLink').href = feedLink;
        setFeedToolTip();
    }

    function changeUILanguage() {
      window.modalUILanguageSelector.close();
      const s = document.getElementById("ui_language");
      const lang = s.options[s.selectedIndex].value;
      localStorage.setItem('userlang', lang);
      window.location.reload();
    }

    function queryUrlBuilder() {
        let url = `${root}/catalog/v2/entries?`;
        url += Object.keys(incrementalLoadingParams).map(key => `${key}=${incrementalLoadingParams[key]}`).join("&");
        params.forEach((value, key) => {url+= value ? `&${key}=${value}` : ''});
        return (url);
    }

    function setCookie(cookieName, cookieValue, ttl) {
        let exp = "";
        if ( ttl ) {
          const date = new Date();
          date.setTime(date.getTime() + ttl);
          exp = `expires=${date.toUTCString()};`;
        }
        document.cookie = `${cookieName}=${cookieValue};${exp}sameSite=Strict`;
    }

    function getCookie(cookieName) {
        const name = cookieName + "=";
        let result;
        decodeURIComponent(document.cookie).split('; ').forEach(val => {
            if (val.indexOf(name) === 0) {
                result = val.substring(name.length);
            }
        });
        return result;
    }

    function humanFriendlyNumStr(num, precision) {
        const n = Math.abs(num).toFixed().length;
        return num.toFixed(Math.max(0, precision - n));
    }

    const humanFriendlySize = (fileSize) => {
        if (fileSize === 0) {
            return '';
        }
        const units = ['bytes', 'KiB', 'MiB', 'GiB', 'TiB'];
        let quotient = Math.floor(Math.log2(fileSize) / 10);
        quotient = Math.min(quotient, units.length - 1);
        fileSize /= (1024 ** quotient);
        const fileSizeStr = humanFriendlyNumStr(fileSize, 3);
        return `${fileSizeStr} ${units[quotient]}`;
    };

    const humanFriendlyTitle = (title) => {
        if (typeof title === 'string' && title.length > 0) {
            title = title.replace(/_/g, ' ');
            if (title.length > 0) {
                return htmlEncode(title[0].toUpperCase() + title.slice(1));
            }
        }
        return '';
    }

    // Borrowed from https://stackoverflow.com/a/1912522
    function htmlDecode(input){
      var e = document.createElement('textarea');
      e.innerHTML = input;
      // handle case of empty input
      return e.childNodes.length === 0 ? "" : e.childNodes[0].nodeValue;
    }

    function htmlEncode(str) {
        return str.replace(/[\u00A0-\u9999<>\&]/gim, (i) => `&#${i.charCodeAt(0)};`);
    }

    function viewPortToCount(){
        const zoom = Math.floor((( window.outerWidth - 10 ) / window.innerWidth) * 100);
        return Math.floor((window.innerHeight/(3*zoom) + 1)*(window.innerWidth/(2.5*zoom) + 1));
    }

    function getInnerHtml(node, query) {
        const queryNode = node.querySelector(query);
        return queryNode != null ? queryNode.innerHTML : "";
    }

    function generateTagLink(tagValue) {
        tagValue = tagValue.toLowerCase();
        const tagMessage = $t("filter-by-tag", {TAG: tagValue});
        const spanElement = document.createElement("span");
        spanElement.className = 'tag__link';
        spanElement.setAttribute('aria-label', tagMessage);
        spanElement.setAttribute('title', tagMessage);
        spanElement.setAttribute('data-tag', tagValue);
        spanElement.innerHTML = htmlEncode(tagValue);
        return spanElement.outerHTML;
    }

    function downloadButtonText(zimSize) {
      return $t("download") + (zimSize ? ` - ${zimSize}`: '');
    }

    function downloadButtonHtml(url, zimSize) {
        return url
             ? `<div class="book__download" data-link="${url}" data-size="${zimSize}">
                  <img src="${root}/skin/download-white.svg?KIWIXCACHEID">
                  <span ">${downloadButtonText(zimSize)}</span>
                </div>`
             : '';
    }

    function addBookPreviewLink(html, bookXml) {
        const bookContentLink = bookXml.querySelector('link[type="text/html"]');
        if ( !bookContentLink )
          return html;

        const urlComponents = bookContentLink.getAttribute('href').split('/');
        // bookContentLink URL = ROOT_URL/content/BOOK_NAME
        const bookName = urlComponents.pop();
        urlComponents.pop(); // drop 'content' component
        const viewerLink = urlComponents.join('/') + `/viewer#${bookName}`;

        return `<a class="book__link" href="${viewerLink}" data-hover="Preview">${html}</a>`;
    }

    function generateBookHtml(book, sort = false) {
        let iconUrl;
        book.querySelectorAll('link[rel="http://opds-spec.org/image/thumbnail"]').forEach(link => {
            if (link.getAttribute('type').split(';')[1] == 'width=48' && !iconUrl) {
                iconUrl = link.getAttribute('href');
            }
        });
        const title =  getInnerHtml(book, 'title');
        const description = getInnerHtml(book, 'summary');
        const id = getInnerHtml(book, 'id');
        const langCodesList = getInnerHtml(book, 'language').split(',');
        const langCode = langCodesList.length == 1 ? langCodesList[0] : 'mul';
        let language = languages[langCode];
        if (langCode == 'mul') {
            const mulLangList = langCodesList.filter(x => languages.hasOwnProperty(x)).map(x => languages[x]);
            language = mulLangList.join(', ');
        }
        const tags = htmlDecode(getInnerHtml(book, 'tags'));
        const tagList = tags.split(';').filter(tag => {return !(tag.startsWith('_'))});
        const tagFilterLinks = tagList.map((tagValue) => generateTagLink(tagValue));
        const tagHtml = tagFilterLinks.join(' | ');
        let downloadLink;
        let zimSize = 0;
        try {
            const downloadBookLink = book.querySelector('link[type="application/x-zim"]')
            zimSize = parseInt(downloadBookLink.getAttribute('length'));
            downloadLink = downloadBookLink.getAttribute('href').split('.meta4')[0];
        } catch {
            downloadLink = '';
        }
        const humanFriendlyZimSize = humanFriendlySize(zimSize);

        const divTag = document.createElement('div');
        divTag.setAttribute('class', 'book');
        divTag.setAttribute('data-id', id);
        if (sort) {
            divTag.setAttribute('data-idx', bookOrderMap.get(id));
        }
        const faviconAttr = iconUrl != undefined ? `style="background-image: url('${iconUrl}')"` : '';
        const languageAttr = langCode != '' ? `title="${language}" aria-label="${language}"` : 'style="background-color: transparent"';

        let bookLinkWrapper = `
            <div class="book__link__wrapper">
            <div class="book__icon" ${faviconAttr}></div>
            <div class="book__header">
                <div id="book__title">${title}</div>
            </div>
            <div class="book__description" title="${description}">${description}</div>
            </div>
        `;

        divTag.innerHTML = `
            <div class="book__wrapper">
            ${addBookPreviewLink(bookLinkWrapper, book)}
            <div class="book__meta">
            <div class="book__languageTag" ${languageAttr}>${getLanguageCodeToDisplay(langCode)}</div>
            <div class="book__tags"><div class="book__tags--wrapper">${tagHtml}</div></div>
            </div>
            ${downloadButtonHtml(downloadLink, humanFriendlyZimSize)}
            </div></div>`;
        return divTag;
    }

    function getLanguageCodeToDisplay(langCode3Letter) {
        const langCode2Letter = (Object.keys(iso6391To3).find(key => iso6391To3[key] === langCode3Letter));
        const res = (langCode2Letter != undefined) ? langCode2Letter : langCode3Letter;
        return res.toUpperCase();
    }

    function toggleFooter(show=false) {
        if (show) {
            footer.style.display = 'block';
        } else {
            footer.style.display = 'none';
            fadeOutDiv.style.display = 'block';
        }
    }

    function makeURLSearchString(params, keysToURIEncode) {
        let output = '';
        for (const [key, value] of params.entries()) {
            let finalValue = (keysToURIEncode.indexOf(key) >= 0) ? encodeURIComponent(value) : value;
            output += `&${key}=${finalValue}`;
        }
        // exclude first char so the first params are not prefixed with &
        return output.substring(1);
    }

    /* hack for library.kiwix.org magnet links (created by MirrorBrain)
        See https://github.com/kiwix/container-images/issues/242 */
    async function getFixedMirrorbrainMagnet(magnetLink) {
        // parse as query parameters
        const params = new URLSearchParams(
            magnetLink.replaceAll('&amp;', '&').replace(/^magnet:/, ''));

        const zimUrl = params.get('as'); // as= is fallback URL
        // download metalink to build list of mirrored URLs
        let mirrorUrls = [];

        const metalink = await fetch(`${zimUrl}.meta4`).then(response => {
            return response.ok ? response.text() : '';
        }).catch((_error) => '');
        if (metalink) {
            try {
                const parser = new DOMParser();
                const doc = parser.parseFromString(metalink, "application/xml");
                doc.querySelectorAll("url").forEach((node) => {
                    if (node.hasAttribute("priority")) { // ensures its a mirror link
                        mirrorUrls.push(node.innerHTML);
                    }
                });
            } catch (err) {
                // not a big deal, magnet will only contain primary URL
                console.debug(`Failed to parse mirror links for ${zimUrl}`);
            }
        }

        // set webseed (ws=) URL to primary download URL (redirects to mirror)
        params.set('ws', zimUrl);
        // if we got metalink mirror URLs, append them all
        if (mirrorUrls) {
            mirrorUrls.forEach((url) => {
                params.append('ws', url);
            });
        }

        params.set('xs', `${zimUrl}.torrent`);  // adding xs= to point to torrent URL

        return 'magnet:?' + makeURLSearchString(params, ['ws', 'as', 'dn', 'xs', 'tr']);
    }

    async function getMagnetLink(downloadLink) {
        const magnetUrl = downloadLink + '.magnet';
        const controller = new AbortController();
        setTimeout(() => controller.abort(), 5000);
        const magnetLink = await fetch(magnetUrl, { signal: controller.signal }).then(response => {
            return response.ok ? response.text() : '';
        }).catch((_error) => '');
        if (magnetLink) {
            return await getFixedMirrorbrainMagnet(magnetLink);
        }
        return magnetLink;
    }

    function insertModal(button) {
        const downloadSize = button.getAttribute('data-size');
        const downloadLink = button.getAttribute('data-link');
        button.addEventListener('click', async (event) => {
            event.preventDefault();
            const magnetLink = await getMagnetLink(downloadLink);
            document.body.insertAdjacentHTML('beforeend', `<div class="modal-wrapper">
                <div class="modal">
                    <div class="modal-heading">
                        <div class="modal-title">
                            <div>
                                ${downloadButtonText(downloadSize)}
                            </div>
                        </div>
                        <div onclick="closeModal()" class="modal-close-button">
                            <div>
                                <svg xmlns="http://www.w3.org/2000/svg" width="14" height="14" viewBox="0 0 14 14" fill="none">
                                    <path fill-rule="evenodd" clip-rule="evenodd" d="M13.7071 1.70711C14.0976 1.31658 14.0976
                                    0.683417 13.7071 0.292893C13.3166 -0.0976311 12.6834 -0.0976311 12.2929 0.292893L7 5.58579L1.70711
                                    0.292893C1.31658 -0.0976311 0.683417 -0.0976311 0.292893 0.292893C-0.0976311 0.683417
                                    -0.0976311 1.31658 0.292893 1.70711L5.58579 7L0.292893 12.2929C-0.0976311 12.6834
                                    -0.0976311 13.3166 0.292893 13.7071C0.683417 14.0976 1.31658 14.0976 1.70711 13.7071L7
                                    8.41421L12.2929 13.7071C12.6834 14.0976 13.3166 14.0976 13.7071 13.7071C14.0976 13.3166
                                    14.0976 12.6834 13.7071 12.2929L8.41421 7L13.7071 1.70711Z" fill="black" />
                                </svg>
                            </div>
                        </div>
                    </div>
                    <div class="modal-content">
                        <div class="modal-regular-download">
                            <a href="${downloadLink}" download>
                                <img src="${root}/skin/download.png?KIWIXCACHEID" alt="${$t("direct-download-alt-text")}" />
                                <div>${$t("direct-download-link-text")}</div>
                            </a>
                        </div>
                        <div class="modal-regular-download">
                            <a href="${downloadLink}.sha256" download>
                                <img src="${root}/skin/hash.png?KIWIXCACHEID" alt="${$t("hash-download-alt-text")}" />
                                <div>${$t("hash-download-link-text")}</div>
                            </a>
                        </div>
                        ${magnetLink ?
                        `<div class="modal-regular-download">
                            <a href="${magnetLink}" target="_blank">
                                <img src="${root}/skin/magnet.png?KIWIXCACHEID" alt="${$t("magnet-alt-text")}" />
                                <div>${$t("magnet-link-text")}</div>
                            </a>
                        </div>` : ``}
                        <div class="modal-regular-download">
                            <a href="${downloadLink}.torrent" download>
                                <img src="${root}/skin/bittorrent.png?KIWIXCACHEID" alt="${$t("torrent-download-alt-text")}" />
                                <div>${$t("torrent-download-link-text")}</div>
                            </a>
                        </div>
                    </div>
                </div>
            </div>`);
        })
    }

    async function getBookCount(query) {
        const url = `${root}/catalog/v2/entries?${query}&count=0`;
        return await fetch(url).then(async (resp) => {
            const data = new window.DOMParser().parseFromString(await resp.text(), 'application/xml');
            return parseInt(data.querySelector('totalResults').innerHTML);
        });
    }

    async function loadBooks() {
        loader.style.display = 'block';
        return await fetch(queryUrlBuilder()).then(async (resp) => {
            const data = new window.DOMParser().parseFromString(await resp.text(), 'application/xml');
            const books = data.querySelectorAll('entry');
            books.forEach((book, idx) => {
                bookOrderMap.set(getInnerHtml(book, 'id'), idx);
            });
            incrementalLoadingParams.start += books.length;
            const results = parseInt(data.querySelector('totalResults').innerHTML)
            if (results === bookOrderMap.size) {
                incrementalLoadingParams.count = 0;
                toggleFooter(true);
            } else {
                toggleFooter();
            }
            const text = results
                       ? $t("count-of-matching-books", {COUNT: results})
                       : '';
            document.querySelector('.kiwixHomeBody__results').innerHTML = text;
            loader.style.display = 'none';
            return books;
        });
    }

    async function loadAndDisplayOptions(nodeQuery, query, valueEntryNode) {
        await fetch(query).then(async (resp) => {
            const data = new window.DOMParser().parseFromString(await resp.text(), 'application/xml');
            let optionStr = '';
            data.querySelectorAll('entry').forEach(entry => {
                const title = getInnerHtml(entry, 'title');
                const value = getInnerHtml(entry, valueEntryNode);
                const hfTitle = humanFriendlyTitle(title);
                if (valueEntryNode == 'language') {
                    languages[value] = hfTitle;
                }
                optionStr += (hfTitle != '') ? `<option value="${value}">${hfTitle}</option>` : '';
            });
            document.querySelector(nodeQuery).innerHTML += optionStr;
        });
    }

    function setNoResultsContent() {
        const kiwixHomeBody = document.querySelector('.kiwixHomeBody');
        const divTag = document.createElement('div');
        divTag.setAttribute('class', 'noResults');
        divTag.innerHTML = $t("welcome-page-overzealous-filter", {URL: '#lang='});
        kiwixHomeBody.append(divTag);
        kiwixHomeBody.setAttribute('style', 'display: flex; justify-content: center; align-items: center');
        loader.setAttribute('style', 'position: absolute; top: 50%');
    }

    function checkAndInjectEmptyMessage() {
        const kiwixHomeBody = document.querySelector('.kiwixHomeBody');
        if (!bookOrderMap.size) {
            if (!noResultInjected) {
                noResultInjected = true;
                iso.remove(document.getElementsByClassName('book__list')[0].getElementsByTagName('div'));
                iso.layout();
                setTimeout(setNoResultsContent, 300);
            }
            return true;
        } else if (noResultInjected) {
            noResultInjected = false;
            document.getElementsByClassName('noResults')[0].remove();
            kiwixHomeBody.removeAttribute('style');
        }
        loader.removeAttribute('style');
        return false;
    }

    async function loadAndDisplayBooks(sort = false) {
        if (isFetching) return;
        isFetching = true;
        await loadAndDisplayBooksUnguarded(sort);
        isFetching = false;
    }

    async function loadAndDisplayBooksUnguarded(sort) {
        let books = await loadBooks();
        if (checkAndInjectEmptyMessage()) {return}
        const booksToFilter = new Set();
        const booksToDelete = new Set();
        iso.arrange({
            filter: function (elem) {
                const id = elem.getAttribute('data-id');
                const retVal = bookOrderMap.has(id);
                if (retVal) {
                    booksToFilter.add(id);
                    if (sort) {
                        elem.setAttribute('data-idx', bookOrderMap.get(id));
                        iso.updateSortData(elem);
                    }
                } else {
                    booksToDelete.add(elem);
                }
                return retVal;
            }
        });
        books = [...books].filter((book) => {return !booksToFilter.has(getInnerHtml(book, 'id'))});
        booksToDelete.forEach(book => {iso.remove(book);});
        books.forEach((book) => {
            iso.insert(generateBookHtml(book, sort))
            const downloadButton = document.querySelector(`[data-id="${getInnerHtml(book, 'id')}"] .book__download`);
            if (downloadButton) {
                insertModal(downloadButton);
            }
        });
        refreshTagLinks();
    }

    async function resetAndFilter(filterType = '', filterValue = '') {
        isFetching = false;
        incrementalLoadingParams.start = 0;
        incrementalLoadingParams.count = viewPortToCount();
        fadeOutDiv.style.display = 'none';
        bookOrderMap.clear();
        params = new FragmentParams(window.location.hash);
        if (filterType) {
            params.set(filterType, filterValue);
            window.history.pushState({}, null, `#${params.toString()}`);
            setCookie(filterCookieName, params.toString(), oneDayDelta);
        }
        updateFilterColors();
        updateFeedLink();
        await loadAndDisplayBooks(true);
    }

    window.addEventListener('popstate', async () => {
        await resetAndFilter();
        updateVisibleParams();
    });

    async function loadSubset() {
        if (window.innerHeight + window.scrollY >= (document.body.offsetHeight * 0.98)) {
            if (incrementalLoadingParams.count) {
                loadAndDisplayBooks();
            }
            else {
                fadeOutDiv.style.display = 'none';
            }
        }
    }

    function updateFilterColors() {
        document.querySelectorAll('.filter').forEach(filter => {
            if (filter.value) {
                filter.style = 'background-color: #858585; color: #fff';
            } else {
                filter.style = 'background-color: #ffffff; color: black';
            }
        });
    }

    function addTagElement(tagValue, resetFilter) {
        const tagElement = document.getElementsByClassName('tagFilterLabel')[0];
        tagElement.style.display = 'inline-block';
        tagElement.innerHTML = htmlEncode(tagValue);
        const tagMessage = $t("stop-filtering-by-tag", {TAG: tagValue});
        tagElement.setAttribute('aria-label', tagMessage);
        tagElement.setAttribute('title', tagMessage);
        if (resetFilter)
            resetAndFilter('tag', tagValue);
    }

    function refreshTagLinks() {
        const tagLinks = document.getElementsByClassName('tag__link');
        [...tagLinks].forEach(elem => {
            if (!elem.getAttribute('click-listener')) {
                elem.addEventListener('click', () => addTagElement(elem.dataset.tag, true));
                elem.setAttribute('click-listener', 'true');
            }
        });
    }

    function removeTagElement(resetFilter) {
        const tagElement = document.getElementsByClassName('tagFilterLabel')[0];
        tagElement.style.display = 'none';
        if (resetFilter)
            resetAndFilter('tag', '');
    }

    function updateVisibleParams() {
        document.querySelectorAll('.filter').forEach(filter => {filter.value = params.get(filter.name) || ''});
        updateFilterColors();
        const tagKey = params.get('tag');
        if (tagKey !== null && tagKey.trim() !== '') {
            addTagElement(tagKey, false);
        } else {
            removeTagElement(false);
        }
    }

    function updateNavVisibilityState() {
        const st = window.scrollY;
        const enableAutoHiding = document.body.clientWidth < 590;
        if ((Math.abs(previousScrollTop - st) <= 5) || !enableAutoHiding)
            return;
        const kiwixNav = document.querySelector('.kiwixNav');
        if (st > previousScrollTop) {
            kiwixNav.style.position = 'fixed';
            kiwixNav.style.top = '-100%';
        } else {
            kiwixNav.style.position = 'sticky';
            kiwixNav.style.top = '0';
        }
        previousScrollTop = st;
    }

    window.addEventListener('resize', (event) => {
        if (timer) {clearTimeout(timer)}
        timer = setTimeout(() => {
            incrementalLoadingParams.count = incrementalLoadingParams.count && viewPortToCount();
            loadSubset();
        }, 100, event);
    });

    window.addEventListener('scroll', loadSubset);

    window.addEventListener('keydown', function (event) {
        if (event.key === "Escape" ) {
            closeModal();
        }
    });

    window.addEventListener('hashchange', () => resetAndFilter());

    function setFeedToolTip() {
        const feedLogoElem = document.getElementById('feedLogo');
        const libraryOpdsFeedHint = opdsFeedHintByParams();
        for (const attr of ["alt", "aria-label", "title"] ) {
            feedLogoElem.setAttribute(attr, libraryOpdsFeedHint);
        }
    }

    function opdsFeedHintByParams() {
        const paramObj = {};
        const inputParams = new FragmentParams(window.location.hash);
        for (const [key, value] of inputParams) {
            if ( value != '' ) {
                paramObj[key.toUpperCase()] = value;
            }
        }
        if (!paramObj.LANG && !paramObj.CATEGORY && !paramObj.TAG && !paramObj.Q) {
            return $t('library-opds-feed-all-entries');
        }
        return $t('library-opds-feed-parameterised', paramObj);
    }

    function updateUIText() {
        footer.innerHTML = $t("powered-by-kiwix-html");
        const searchText = $t("search");
        document.getElementById('searchFilter').placeholder = searchText;
        document.getElementById('searchButton').value = searchText;
        document.getElementById('categoryFilter').children[0].innerHTML = $t("book-filtering-all-categories");
        document.getElementById('languageFilter').children[0].innerHTML = $t("book-filtering-all-languages");
        setFeedToolTip();
    }

    async function onload() {
        initUILanguageSelector(getUserLanguage(), changeUILanguage);
        iso = new Isotope( '.book__list', {
            itemSelector: '.book',
            getSortData:{
                weight: function( itemElem ) {
                    const index = itemElem.getAttribute('data-idx');
                    return index ? parseInt(index) : Infinity;
                }
            },
            sortBy: 'weight',
            layoutMode: 'masonry',
            masonry: {
                fitWidth: true
            }
        });
        footer = document.getElementById('kiwixfooter');
        updateUIText();
        fadeOutDiv = document.getElementById('fadeOut');
        loader = document.querySelector('.loader');
        await loadAndDisplayOptions('#languageFilter', `${root}/catalog/v2/languages`, 'language');
        await loadAndDisplayOptions('#categoryFilter', `${root}/catalog/v2/categories`, 'title');
        await loadAndDisplayBooks();
        document.querySelectorAll('.filter').forEach(filter => {
            filter.addEventListener('change', () => {resetAndFilter(filter.name, filter.value)});
        });
        const tagElement = document.getElementsByClassName('tagFilterLabel')[0];
        tagElement.addEventListener('click', () => removeTagElement(true));
        if (filters) {
            const currentLink = window.location.hash;
            const newLink = `#${params.toString()}`;
            if (currentLink != newLink) {
                window.history.pushState({}, null, newLink);
            }
        }
        updateVisibleParams();
        document.getElementById('kiwixSearchForm').onsubmit = (event) => {event.preventDefault()};
        if (!window.location.hash) {
            const browserLang = navigator.language.split('-')[0];
            const langFilter = document.getElementById('languageFilter');
            const lang = browserLang.length === 3 ? browserLang : iso6391To3[browserLang];
            if (await getBookCount(`lang=${lang}`)) {
                langFilter.value = lang;
                langFilter.dispatchEvent(new Event('change'));
            }
        }
        updateFeedLink();
        setCookie(filterCookieName, params.toString(), oneDayDelta);
        setInterval(updateNavVisibilityState, 250);
    };

    window.onload = () => { setUserLanguage(getUserLanguage(), onload); }
})();

