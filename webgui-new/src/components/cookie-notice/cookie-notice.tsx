import React, { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { getStore, setStore } from 'utils/store';
import ReactGA from 'react-ga4';
import {
  Paper,
  Typography,
  Button,
  IconButton,
  Snackbar,
  Dialog,
  DialogTitle,
  DialogContent,
  DialogActions,
  TableContainer,
  TableRow,
  TableCell,
  Table,
  TableHead,
  TableBody,
  Link,
} from '@mui/material';
import CloseIcon from '@mui/icons-material/Close';
import { useRouter } from 'next/router';

export const CookieNotice = (): JSX.Element => {
  const { t } = useTranslation();
  const router = useRouter();
  const [isCookieConsent, setIsCookieConsent] = useState<boolean | undefined>(undefined);
  const [gaTrackingCode, setGaTrackingCode] = useState<string | undefined>(undefined);
  const [openPolicyModal, setOpenPolicyModal] = useState(false);
  const [showNoticeSnackbar, setShowNoticeSnackbar] = useState(true);

  useEffect(() => {
    const fetchGaTrackingCode = async () => {
      try {
        const res = await fetch(`/ga.txt`);
        const gaCode = await res.text();
        setGaTrackingCode(gaCode);
      } catch (e) {
        setGaTrackingCode(undefined);
      }
      const store = getStore();
      setIsCookieConsent(store.storedCookieConsent);
    };
    fetchGaTrackingCode();
  }, []);

  useEffect(() => {
    if (!isCookieConsent || !gaTrackingCode) {
      ReactGA.reset();
      return;
    }
    if (!ReactGA.isInitialized) {
      ReactGA.initialize(gaTrackingCode);
      console.log(`Google Analytics initialized - ${gaTrackingCode}`);
    }

    const handleRouteChange = (url: string) => {
      ReactGA.send({ hitType: 'pageview', page: url, title: window.document.title });
    };
    router.events.on('routeChangeComplete', handleRouteChange);

    return () => {
      router.events.off('routeChangeComplete', handleRouteChange);
    };
  }, [isCookieConsent, gaTrackingCode, router.events]);

  const handleCookieAccept = () => {
    setIsCookieConsent(true);
    setShowNoticeSnackbar(false);
    setStore({ storedCookieConsent: true });
  };

  if (!isCookieConsent && gaTrackingCode)
    return (
      <>
        <Snackbar
          open={showNoticeSnackbar}
          autoHideDuration={null}
          anchorOrigin={{ vertical: 'bottom', horizontal: 'right' }}
        >
          <Paper
            sx={{
              p: 2,
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'space-between',
            }}
          >
            <Typography variant="body2">
              {t('cookie.INTRO_TEXT')}
              <Button color="primary" size="small" onClick={() => setOpenPolicyModal(true)}>
                {t('cookie.LEARN_MORE')}
              </Button>
            </Typography>
            <Button
              color="primary"
              size="small"
              variant="contained"
              onClick={() => {
                handleCookieAccept();
              }}
            >
              {t('cookie.ACCEPT')}
            </Button>
            <IconButton
              size="small"
              color="inherit"
              sx={{ ml: 2 }}
              onClick={() => setShowNoticeSnackbar(false)}
            >
              <CloseIcon fontSize="small" />
            </IconButton>
          </Paper>
        </Snackbar>
        <Dialog open={openPolicyModal} maxWidth="lg" onClose={() => setOpenPolicyModal(false)}>
          <DialogTitle>{t('cookiePolicy.TITLE')}</DialogTitle>
          <DialogContent>
            <>
              <Typography sx={{ marginBottom: '0.8rem' }} variant="body1">
                {t('cookiePolicy.sections.cookies.WHAT_COOKIES')}
              </Typography>
              <Typography sx={{ marginBottom: '0.8rem' }} variant="body2">
                {t('cookiePolicy.sections.cookies.WHAT_COOKIES_DESCRIPTION')}
              </Typography>
              <Typography sx={{ marginBottom: '0.8rem' }} variant="body1">
                {t('cookiePolicy.sections.cookies.HOW_USE_TITLE')}
              </Typography>
              <TableContainer component={Paper} sx={{ marginBottom: '0.8rem' }}>
                <Table sx={{ minWidth: 650 }} aria-label="cookie-table">
                  <TableHead>
                    <TableRow>
                      <TableCell>{t('cookiePolicy.sections.cookies.SERVICE')}</TableCell>
                      <TableCell>{t('cookiePolicy.sections.cookies.COOKIE_NAMES')}</TableCell>
                      <TableCell>{t('cookiePolicy.sections.cookies.DURATION')}</TableCell>
                      <TableCell>{t('cookiePolicy.sections.cookies.PURPOSE')}</TableCell>
                      <TableCell>{t('cookiePolicy.sections.cookies.MORE_INFORMATION')}</TableCell>
                    </TableRow>
                  </TableHead>
                  <TableBody>
                    <TableRow key={1} sx={{ '&:last-child td, &:last-child th': { border: 0 } }}>
                      <TableCell component="th" scope="row">
                        <b>{t('cookiePolicy.sections.cookies.googleAnalytics.SERVICE')}</b>
                      </TableCell>
                      <TableCell>
                        <b>{t('cookiePolicy.sections.cookies.googleAnalytics.COOKIE_NAMES')}</b>
                      </TableCell>
                      <TableCell>
                        <b>{t('cookiePolicy.sections.cookies.googleAnalytics.DURATION')}</b>
                      </TableCell>
                      <TableCell>
                        <b>{t('cookiePolicy.sections.cookies.googleAnalytics.PURPOSE')}</b>
                      </TableCell>
                      <TableCell>
                        <Link
                          target="_blank"
                          rel="noreferrer"
                          href={t('cookiePolicy.sections.cookies.googleAnalytics.MORE_INFORMATION')}
                        >
                          <b>{t('cookiePolicy.sections.cookies.MORE_INFORMATION')}</b>
                        </Link>
                      </TableCell>
                    </TableRow>
                  </TableBody>
                </Table>
              </TableContainer>
              <Typography variant="body2">
                {t('cookiePolicy.sections.cookies.CONCLUSION')}
              </Typography>
            </>
          </DialogContent>
          <DialogActions>
            <Button onClick={() => setOpenPolicyModal(false)} color="primary"></Button>
          </DialogActions>
        </Dialog>
      </>
    );

  return <></>;
};
