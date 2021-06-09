set pagesize 0
set termout off
set long 10000000
spool timm.etis
select document from document_all where rowid = '&1';
spool off
quit