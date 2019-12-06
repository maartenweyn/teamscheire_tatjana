cordova.define("cordova-plugin-advanced-http.messages", function(require, exports, module) {
module.exports = {
  ADDING_COOKIES_NOT_SUPPORTED: 'advanced-http: "setHeader" does not support adding cookies, please use "setCookie" function instead',
  DATA_TYPE_MISMATCH: 'advanced-http: "data" argument supports only following data types:',
  INVALID_CLIENT_AUTH_ALIAS: 'advanced-http: invalid client certificate alias, needs to be a string or undefined',
  INVALID_CLIENT_AUTH_MODE: 'advanced-http: invalid client certificate authentication mode, supported modes are:',
  INVALID_CLIENT_AUTH_OPTIONS: 'advanced-http: invalid client certificate authentication options, needs to be an object',
  INVALID_CLIENT_AUTH_PKCS_PASSWORD: 'advanced-http: invalid PKCS12 container password, needs to be a string',
  INVALID_CLIENT_AUTH_RAW_PKCS: 'advanced-http: invalid PKCS12 container, needs to be an array buffer',
  INVALID_DATA_SERIALIZER: 'advanced-http: invalid serializer, supported serializers are:',
  INVALID_FOLLOW_REDIRECT_VALUE: 'advanced-http: invalid follow redirect value, needs to be a boolean value',
  INVALID_HEADERS_VALUE: 'advanced-http: header values must be strings',
  INVALID_HTTP_METHOD: 'advanced-http: invalid HTTP method, supported methods are:',
  INVALID_PARAMS_VALUE: 'advanced-http: invalid params object, needs to be an object with strings',
  INVALID_RESPONSE_TYPE: 'advanced-http: invalid response type, supported types are:',
  INVALID_SSL_CERT_MODE: 'advanced-http: invalid SSL cert mode, supported modes are:',
  INVALID_TIMEOUT_VALUE: 'advanced-http: invalid timeout value, needs to be a positive numeric value',
  MANDATORY_FAIL: 'advanced-http: missing mandatory "onFail" callback function',
  MANDATORY_SUCCESS: 'advanced-http: missing mandatory "onSuccess" callback function',
};

});
