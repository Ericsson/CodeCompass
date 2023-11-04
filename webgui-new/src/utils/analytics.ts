import ReactGA from 'react-ga4';

type EventType = {
  event_action: string;
  event_category: string;
  event_label?: string;
};

export const sendGAEvent = ({
  event_action,
  event_category,
  event_label = 'undefined',
}: EventType) => {
  if (!ReactGA.isInitialized) {
    return;
  }
  // We have to use this signature to prevent ReactGA from making the starting letters automatically uppercase.
  ReactGA.event(event_action, { event_category, event_label });
};
