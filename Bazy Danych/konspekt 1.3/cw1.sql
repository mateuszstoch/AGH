-- zadanie 1
select t.title ,t.title_no   from title t 

--zadanie 2
select t.title  from title t where t.title_no = 10

--zadanie 3
select t.title_no,t.author  from title t where t.author in ('Charles Dickens','Jane Austen')