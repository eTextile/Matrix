XMLElement xml;

String code;

// This is where you enter your Oauth info
String OAuthConsumerKey = "";
String OAuthConsumerSecret = "";
// This is where you enter your Access Token info
String AccessToken = "";
String AccessTokenSecret = "";


Twitter twitter = new TwitterFactory().getInstance();
RequestToken requestToken;

/////////////////// Read codes on XML config file  
void codeSetup() {

  xml = new XMLElement(this, "config.xml");
  int numCodes = xml.getChildCount();

  for (int i=0; i<numCodes; i++) {
    XMLElement kid = xml.getChild(i);

    if (i == 0) {
      code = kid.getContent();
      OAuthConsumerKey = code;
      if (DEBUG_twitter_id) println( "OAuthConsumerKey : " + OAuthConsumerKey );
    }
    if (i == 1) {
      code = kid.getContent();
      OAuthConsumerSecret = code;
      if (DEBUG_twitter_id) println( "OAuthConsumerSecret : " + OAuthConsumerSecret );
    }
    if (i == 2) {
      code = kid.getContent();
      AccessToken = code;
      if (DEBUG_twitter_id) println( "AccessToken : " + AccessToken );
    }
    if (i == 3) {
      code = kid.getContent();
      AccessTokenSecret = code;
      if (DEBUG_twitter_id) println( "AccessTokenSecret : " + AccessTokenSecret );
    }
  }
}

/////////////////// Initial connection
void connectTwitter() {
  twitter.setOAuthConsumer(OAuthConsumerKey, OAuthConsumerSecret);
  AccessToken accessToken = loadAccessToken();
  twitter.setOAuthAccessToken(accessToken);
}

/////////////////// Sending a tweet
void sendTweet(String t) {
  try {
    Status status = twitter.updateStatus(t);
    println("Successfully updated the status to [" + status.getText() + "].");
  } 
  catch(TwitterException e) { 
    println("Send tweet: " + e + " Status code: " + e.getStatusCode());
  }
}

/////////////////// Loading up the access token
private AccessToken loadAccessToken() {
  return new AccessToken(AccessToken, AccessTokenSecret);
}

