--zadanie 1
select t.title_no , t.title  from title t where t.title like '%adventure%'

--zadanie 2
select l.member_no ,l.fine_paid  from loanhist l 
where isnull(l.fine_paid,0) > 0 and year(l.in_date ) = 2001 and month(l.in_date ) = 11 

--zadanie 3
select distinct a.state , a.city  from adult a 

--zadanie 4
select t.title from title t ORDER by t.title  