--zadanie 1
select e.EmployeeID ,e.ReportsTo  from Employees e 

--zadanie 2
select e.EmployeeID , e.ReportsTo  from Employees e 
left join Employees e2 on e.EmployeeID = e2.ReportsTo 
where e2.EmployeeID is Null

--zadanie 3
select distinct e.EmployeeID from Employees e 
inner join Employees e2 on e.EmployeeID = e2.ReportsTo 