import mustache from '../skin/mustache.min.js?KIWIXCACHEID'

const Translations = {
  defaultLanguage: null,
  currentLanguage: null,
  promises: {},
  data: {},

  load: function(lang, asDefault=false) {
    if ( asDefault ) {
      this.defaultLanguage = lang;
      this.loadTranslationsJSON(lang);
    } else {
      this.currentLanguage = lang;
      if ( lang != this.defaultLanguage ) {
        this.loadTranslationsJSON(lang);
      }
    }
  },

  loadTranslationsJSON: function(lang) {
    if ( this.promises[lang] )
      return;

    const errorMsg = `Error loading translations for language '${lang}': `;
    const translationJsonUrl = import.meta.resolve(`./i18n/${lang}.json`);
    this.promises[lang] = fetch(translationJsonUrl).then(async (resp) => {
      if ( resp.ok ) {
        this.data[lang] = JSON.parse(await resp.text());
      } else {
        console.log(errorMsg + resp.statusText);
      }
    }).catch((err) => {
      console.log(errorMsg + err);
    });
  },

  whenReady: function(callback) {
    const defaultLangPromise = this.promises[this.defaultLanguage];
    const currentLangPromise = this.promises[this.currentLanguage];
    Promise.all([defaultLangPromise, currentLangPromise]).then(callback);
  },

  get: function(msgId) {
    const activeTranslation = this.data[this.currentLanguage];

    const r = activeTranslation && activeTranslation[msgId];
    if ( r )
      return r;

    const defaultMsgs = this.data[this.defaultLanguage];
    if ( defaultMsgs )
      return defaultMsgs[msgId];

    throw "Translations are not loaded";
  }
}


function $t(msgId, params={}) {
  try {
    const msgTemplate = Translations.get(msgId);
    if ( ! msgTemplate ) {
      return "Invalid message id: " + msgId;
    }

    return mustache.render(msgTemplate, params);
  } catch (err) {
    return "ERROR: " + err;
  }
}

const I18n = {
  instantiateParameterizedMessages: function(data) {
    if ( data.__proto__ == Array.prototype ) {
      const result = [];
      for ( const x of data ) {
        result.push(this.instantiateParameterizedMessages(x));
      }
      return result;
    } else if ( data.__proto__ == Object.prototype ) {
      const msgId = data.msgid;
      const msgParams = data.params;
      if ( msgId && msgId.__proto__ == String.prototype && msgParams && msgParams.__proto__ == Object.prototype ) {
        return $t(msgId, msgParams);
      } else {
        const result = {};
        for ( const p in data ) {
          result[p] = this.instantiateParameterizedMessages(data[p]);
        }
        return result;
      }
    } else {
      return data;
    }
  },

  render: function (template, params) {
    params = this.instantiateParameterizedMessages(params);
    return mustache.render(template, params);
  }
}

const DEFAULT_UI_LANGUAGE = 'en';

Translations.load(DEFAULT_UI_LANGUAGE, /*asDefault=*/true);

// Below function selects the most suitable UI language from the list
// of preferred languages in browser preferences and available translations.
// Since, unlike Accept-Language header, navigator.languages doesn't contain
// qvalues, they are computed using the same algorithm as in Firefox 121
function getDefaultUserLanguage() {
  const mostSuitableLang = { code: DEFAULT_UI_LANGUAGE, score: 0 }
  const n = navigator.languages.length;
  for (const lang of uiLanguages ) {
    const rank = navigator.languages.indexOf(lang.iso_code);
    if ( rank >= 0 ) {
      const qvalue = Math.round(10*(1 - rank/n))/10;
      const score = qvalue * lang.translation_count;
      if ( score > mostSuitableLang.score ) {
        mostSuitableLang.code = lang.iso_code;
        mostSuitableLang.score = score;
      }
    }
  }
  return mostSuitableLang.code;
}

function getUserLanguage() {
  return new URLSearchParams(window.location.search).get('userlang')
      || window.localStorage.getItem('userlang')
      || getDefaultUserLanguage();
}

function setUserLanguage(lang, callback) {
  window.localStorage.setItem('userlang', lang);
  Translations.load(lang);
  Translations.whenReady(callback);
}

function createModalUILanguageSelector() {
  document.body.insertAdjacentHTML('beforeend',
    `<div id="uiLanguageSelector" class="modal-wrapper">
      <div class="modal">
        <div class="modal-heading">
          <div class="modal-title">
            <div>
                Select UI language
            </div>
          </div>
          <div onclick="window.modalUILanguageSelector.close()" class="modal-close-button">
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
          <select id="ui_language"></select>
        </div>
      </div>
    </div>`);

  window.modalUILanguageSelector = {
    show: () => {
      document.getElementById('uiLanguageSelector').style.display = 'flex';
    },

    close: () => {
      document.getElementById('uiLanguageSelector').style.display = 'none';
    }
  };
}

function initUILanguageSelector(activeLanguage, languageChangeCallback) {
  if ( document.getElementById("ui_language") == null ) {
    createModalUILanguageSelector();
  }
  const languageSelector = document.getElementById("ui_language");
  for (const lang of uiLanguages ) {
    const is_selected = lang.iso_code == activeLanguage;
    languageSelector.appendChild(new Option(lang.self_name, lang.iso_code, is_selected, is_selected));
  }
  languageSelector.onchange = languageChangeCallback;
}

function parseDom(html) {
  const domParser = new DOMParser();
  return domParser.parseFromString(html, "text/html").documentElement;
}

function translatePageInWindow(w) {
  if ( w.KIWIX_RESPONSE_TEMPLATE && w.KIWIX_RESPONSE_DATA ) {
    const template = parseDom(w.KIWIX_RESPONSE_TEMPLATE).textContent;

    // w.KIWIX_RESPONSE_DATA may belong to a different context and running
    // I18n.render() on it directly won't work correctly
    // because the type checks (obj.__proto__ == ???.prototype) in
    // I18n.instantiateParameterizedMessages() will fail (String.prototype
    // refers to different objects in different contexts).
    // Work arround that issue by copying the object into our context.
    const params = JSON.parse(JSON.stringify(w.KIWIX_RESPONSE_DATA));

    const newHtml = I18n.render(template, params);
    w.document.documentElement.innerHTML = parseDom(newHtml).innerHTML;
  }
}

function translateSelf() {
  if ( window.KIWIX_RESPONSE_TEMPLATE && window.KIWIX_RESPONSE_DATA ) {
    setUserLanguage(getUserLanguage(), () => {
      translatePageInWindow(window)
    });
  }
};

window.$t = $t;
window.getUserLanguage = getUserLanguage;
window.setUserLanguage = setUserLanguage;
window.initUILanguageSelector = initUILanguageSelector;
window.translatePageInWindow = translatePageInWindow;
window.I18n = I18n;
window.addEventListener('load', translateSelf);
